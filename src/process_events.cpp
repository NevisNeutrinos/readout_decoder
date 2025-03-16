//
// Created by Jon Sensenig on 3/14/25.
//

#include "process_events.h"
#include "charge_light_decoder.h"
#include <cerrno>

namespace py = pybind11;

ProcessEvents::ProcessEvents(): charge_light_decoder_(nullptr) {
    charge_light_decoder_ = new decoder::Decoder;
    event_dict_["Light"] = py::dict();
    event_dict_["Charge"] = py::dict();
}

ProcessEvents::~ProcessEvents() {

    if (data_file_) {
        std::cout << "Closing data file!" << std::endl;
        fclose(data_file_);
        data_file_ = nullptr;
    }
    delete charge_light_decoder_;
}

bool ProcessEvents::OpenFile(const std::string &file_name) {

    // In case there's a file already open
    word_idx_ = 0;
    if (data_file_) {
        std::cout << "Closing data file!" << std::endl;
        fclose(data_file_);
        data_file_ = nullptr;
    }

    std::cout << "Opening file " << file_name << std::endl;
    data_file_ = fopen(file_name.c_str(), "rb");
    if(data_file_ == nullptr) {
        std::cerr << "Could not open file: " << file_name << std::endl;
        return false;
    }

    // Get file size
    const long lCurPos = ftell(data_file_);
    fseek(data_file_, 0, 2);
    const long fileSize = ftell(data_file_);
    fseek(data_file_, lCurPos, 0);
    file_num_words_ = fileSize / sizeof(uint32_t);
    std::cout << "File size: " << fileSize << std::endl;

    // allocate space in the buffer for the whole file
    file_buffer_ = new int[file_num_words_];
    std::cout << "Allocated file buffer.." << std::endl;

    fread(file_buffer_, fileSize, 1, data_file_);
    if (ferror(data_file_)) {
        std::cerr << "Error reading file: " << file_name << std::endl;
        std::cerr << "Error code: [" << errno << "]" << std::endl;
        return false;
    }
    std::cout << "Read file.." << std::endl;
    return true;
}

bool ProcessEvents::GetEvent() {

    bool read_charge_channel = false;
    bool read_light_channel = false;
    bool light_word_header_done = false;
    bool end_fem = false;
    bool is_light = false;

    // This will run from the start of the event until
    // end of event marker is reached or if all words are
    // read from file.

    while (word_idx_ < file_num_words_) {
        const uint32_t word_32 = file_buffer_[word_idx_];
        word_idx_++;
        if (decoder::Decoder::IsEventStart(word_32)) {
            // Reset the FEM header decoder state machine
            charge_light_decoder_->HeaderWord = 0;
            continue;
        } if (decoder::Decoder::IsEventEnd(word_32)) {
            if ((event_number_ % 100) == 0) std::cout << "+++ Event [" << event_number_ << "]" << std::endl;
            event_number_++;
            is_light = charge_light_decoder_->GetSlotNumber() == 16;
            if (end_fem) FillFemDict(is_light);
            end_fem = false;
            return true;
        }

        if (decoder::Decoder::IsHeaderWord(word_32)) {
            // FIXME handle more FEMs
            is_light = charge_light_decoder_->GetSlotNumber() == 16;
            if (end_fem) FillFemDict(is_light);
            end_fem = false;
            charge_light_decoder_->FemHeaderDecode(word_32);
            charge_channel_number_ = 0;
            channel_number_.clear();
            charge_adc_.clear();
            light_adc_.clear();
            light_frame_number_.clear();
            light_sample_number_.clear();
            continue;
        }
        end_fem = true;
        const uint16_t slot_number = charge_light_decoder_->GetSlotNumber();
        for (size_t j = 0; j < 2; j++) {

            // FIXME The 16b words should be aligned as a 32b word at this point but should add check
            // 32b word & 0xFFFF is 1R the 1st word
            // (32b word >> 16) & 0xFFFF is 1L the 2nd word
            uint16_t word = j == 0 ? word_32 & 0xFFFF : (word_32 >> 16) & 0xFFFF;

            if (decoder::Decoder::ChargeChannelStart(word) && slot_number != 16) {
                // std::cout << "ChargeChannel Start " << (word & 0x3F) << "\n";
                read_charge_channel = true;
            }
            else if (decoder::Decoder::ChargeChannelEnd(word) && slot_number != 16) {
                read_charge_channel = false;
                charge_adc_.push_back(charge_light_decoder_->GetAdcWords());
                charge_light_decoder_->ResetAdcWordVector();
                channel_number_.push_back(charge_channel_number_);
                charge_channel_number_++;
            }
            else if (read_charge_channel) {
                // FIXME, does this work!?
                charge_light_decoder_->DecodeAdcWord(word);
                // charge_light_decoder_->GetChargeAdcChunk<595>(word_idx_, j, file_buffer_, &charge_adc_arr_);
                // j = 1; // we are guaranteed to have the chunk aligned to the 32b word so break the loop
                // break; // we are guaranteed to have the chunk aligned to the 32b word so break the loop
            }
            else if (decoder::Decoder::LightChannelStart(word) && slot_number == 16) {
                read_light_channel = true;
            }
            else if (decoder::Decoder::LightChannelEnd(word) && slot_number == 16) {
                read_light_channel = false;
            }
            else if (read_light_channel) {
                if (!decoder::Decoder::LightChannelIntmed(word)) {
                    // std::cerr << "Unexpected word ID!" << std::endl;
                }
                if (decoder::Decoder::LightRoiHeader1(word) || !light_word_header_done) {
                    light_word_header_done = charge_light_decoder_->FemLightDecode(word);
                }
                else if (decoder::Decoder::LightRoiHeader2(word)) {
                    charge_light_decoder_->DecodeAdcWord(word);
                }
                else if (decoder::Decoder::LightRoiEnd(word)) {
                    charge_light_decoder_->LightWord = 0;
                    light_adc_.push_back(charge_light_decoder_->GetAdcWords());
                    charge_light_decoder_->ResetAdcWordVector();
                    channel_number_.push_back(charge_light_decoder_->GetLightChannel());
                    light_frame_number_.push_back(charge_light_decoder_->GetLightFrameNumber());
                    light_sample_number_.push_back(charge_light_decoder_->GetLightSampleNumber());
                    light_word_header_done = false;
                }
                else {
                    // std::cout << "Unexpected light word! " << (word & 0x3000)  << " "
                    // << light_word_header_done << std::endl;
                }
            }
        }
    }

    if (data_file_ != nullptr) {
        fclose(data_file_);
        data_file_ = nullptr;
    }

    std::cout << "event_number_: " << event_number_ << std::endl;
    return false;
}

bool ProcessEvents::GetNumEvents(int num_events) {

    size_t event_count = 0;
    while (GetEvent() && num_events > event_count) {
        event_count++;
    }

    if (data_file_ != nullptr) {
        fclose(data_file_);
        data_file_ = nullptr;
    }
    return true;
}

void ProcessEvents::FillFemDict(const bool is_light) {
    pybind11::dict fem_dict_;
    fem_dict_["slot_number"] = charge_light_decoder_->GetSlotNumber();
    fem_dict_["num_adc_word"] = charge_light_decoder_->GetNumAdcWords();
    fem_dict_["event_number"] = charge_light_decoder_->GetEventNumber();
    fem_dict_["event_frame_number"] = charge_light_decoder_->GetEventFrameNumber();
    fem_dict_["trigger_frame_number"] = charge_light_decoder_->GetTriggerFrameNumber();
    fem_dict_["check_sum"] = charge_light_decoder_->GetCheckSum();
    fem_dict_["trigger_sample"] = charge_light_decoder_->GetTriggerSample();

    if (is_light) {
        fem_dict_["channel"] = vector_to_numpy_array_1d(channel_number_);
        fem_dict_["light_frame_number"] = vector_to_numpy_array_1d(light_frame_number_);
        fem_dict_["light_readout_sample"] = vector_to_numpy_array_1d(light_sample_number_);
        fem_dict_["adc_words"] = vector_to_numpy_array_2d(light_adc_);
        fem_dict_["adc_words_reco"] = ReconstructLightWaveforms();
        fem_dict_["adc_axis_reco"] = ReconstructLightAxis();
        event_dict_["Light"][fem_dict_["slot_number"]] = fem_dict_;
    } else {
        fem_dict_["channel"] = vector_to_numpy_array_1d(channel_number_);
        fem_dict_["adc_words"] = vector_to_numpy_array_2d(charge_adc_);
        // fem_dict_["adc_words"] = to_numpy_array_2d(charge_adc_arr_);
        event_dict_["Charge"][fem_dict_["slot_number"]] = fem_dict_;
    }
}

py::array_t<double> ProcessEvents::ReconstructLightAxis() {
    constexpr int samples_per_frame = 255 * 32; // timesize * 32MHz
    constexpr double light_sample_interval = 15.625;

    const auto min_it = std::min_element(light_frame_number_.begin(), light_frame_number_.end());
    uint16_t min_frame_number = min_it != light_frame_number_.end() ? *min_it : 0;

    uint16_t frame_offset = (charge_light_decoder_->GetTriggerFrameNumber() - min_frame_number) * light_sample_interval;
    uint16_t trigger_index = frame_offset + (charge_light_decoder_->GetTriggerSample() * 32);

    std::array<double, 10 * samples_per_frame> light_axis{};
    double tick_idx = 0;
    for (auto &tick : light_axis) {
        tick = (tick_idx - trigger_index) * light_sample_interval;
        tick_idx++;
    }
    return py::array_t<double>(light_axis.size(), &light_axis[0]);
}

py::array_t<uint16_t> ProcessEvents::ReconstructLightWaveforms() {
    constexpr int samples_per_frame = 255 * 32; // timesize * 32MHz
    std::array<std::array<uint16_t, 10 * samples_per_frame>, 32> channel_full_waveform{};
    for (auto &channel : channel_full_waveform) {
        for (auto &sample : channel) {
            sample = 2048;
        }
    }

    const auto min_it = std::min_element(light_frame_number_.begin(), light_frame_number_.end());
    uint16_t min_frame_number = min_it != light_frame_number_.end() ? *min_it : 0;

    for (size_t roi = 0; roi < channel_number_.size(); roi++) {
        size_t channel = channel_number_.at(roi);
        if (channel > 31) continue;
        size_t frame_offset = (light_frame_number_.at(roi) - min_frame_number) * samples_per_frame;
        size_t start_idx = light_sample_number_.at(roi) + frame_offset;
        size_t end_idx = std::min((start_idx + light_adc_.at(roi).size()), channel_full_waveform.at(channel).size());
        for (size_t idx = start_idx; idx < end_idx; idx++) {
            channel_full_waveform.at(channel).at(idx) = light_adc_.at(roi).at(idx - start_idx);
        }
    }
    return to_numpy_array_2d(channel_full_waveform);
}


