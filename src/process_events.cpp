//
// Created by Jon Sensenig on 3/14/25.
//

#include "process_events.h"
#include "charge_light_decoder.h"
#include <cerrno>


ProcessEvents::ProcessEvents(const uint16_t light_slot): charge_light_decoder_(nullptr), light_slot_(light_slot) {
    charge_light_decoder_ = std::make_unique<decoder::Decoder>();
    // event_dict_["Light"] = py::dict();
    // event_dict_["Charge"] = py::dict();
    channel_full_waveform_.reserve(num_light_channels_);
    channel_full_axis_.reserve(num_light_channels_);
}

ProcessEvents::~ProcessEvents() {

    if (data_file_) {
        std::cout << "Closing data file!" << std::endl;
        file_buffer_.reset(nullptr);
        fclose(data_file_);
        //delete[] data_file_; // FIXME memory freed twice!
        data_file_ = nullptr;
    }
    charge_light_decoder_.reset(nullptr);
}

bool ProcessEvents::OpenFile(const std::string &file_name) {

    // In case there's a file already open
    word_idx_ = 0;
    if (data_file_) {
        file_buffer_.reset(nullptr);
        std::cout << "Closing data file!" << std::endl;
        fclose(data_file_);
        //delete[] data_file_; // FIXME memory freed twice!
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

    file_buffer_ = std::make_unique<int[]>(file_num_words_);
    std::cout << "Allocated file buffer.." << std::endl;

    fread(file_buffer_.get(), fileSize, 1, data_file_);
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

    // This will run from the start of the event until
    // end of event marker is reached or if all words are
    // read from file.

    while (word_idx_ < file_num_words_) {
        const uint32_t word_32 = file_buffer_[word_idx_];
        word_idx_++;
        if (decoder::Decoder::IsEventStart(word_32)) {
            // Reset the FEM header decoder state machine
            ClearFemVectors();
            continue;
        }
        if (decoder::Decoder::IsEventEnd(word_32)) {
            if ((event_number_ % 100) == 0) std::cout << "+++ Event [" << event_number_ << "]" << std::endl;
            event_number_++;
            FillFemDict();
            return true;
        }
        if (decoder::Decoder::IsHeaderWord(word_32)) {
            // returns true when the last FEM header word is reached, so set the FEM data
            if (charge_light_decoder_->FemHeaderDecode(word_32)) SetFemData();
            continue;
        }

        const uint16_t slot_number = charge_light_decoder_->GetSlotNumber();
        for (size_t j = 0; j < 2; j++) {

            // FIXME The 16b words should be aligned as a 32b word at this point but should add check
            // 32b word & 0xFFFF is 1R the 1st word
            // (32b word >> 16) & 0xFFFF is 1L the 2nd word
            uint16_t word = j == 0 ? word_32 & 0xFFFF : (word_32 >> 16) & 0xFFFF;

            if (decoder::Decoder::ChargeChannelStart(word) && !read_charge_channel && slot_number != light_slot_) {
                // std::cout << "ChargeChannel Start " << (word & 0x3F) << "\n";
                read_charge_channel = true;
            }
            else if (decoder::Decoder::ChargeChannelEnd(word) && read_charge_channel && slot_number != light_slot_) {
                read_charge_channel = false;
                charge_adc_.push_back(charge_light_decoder_->GetAdcWords());
                charge_light_decoder_->ResetAdcWordVector();
                charge_channel_.push_back(charge_channel_number_++);
            }
            else if (read_charge_channel) {
                charge_light_decoder_->DecodeAdcWord(word);
                // charge_light_decoder_->GetChargeAdcChunk<595>(word_idx_, j, file_buffer_, &charge_adc_arr_);
                // j = 1; // we are guaranteed to have the chunk aligned to the 32b word so break the loop
                // break; // we are guaranteed to have the chunk aligned to the 32b word so break the loop
            }
            else if (decoder::Decoder::LightChannelStart(word) && slot_number == light_slot_) {
                read_light_channel = true;
            }
            else if (decoder::Decoder::LightChannelEnd(word) && slot_number == light_slot_) {
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
                    light_channel_.push_back(charge_light_decoder_->GetLightChannel());
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

void ProcessEvents::SetFemData() {
    slot_number_v_.push_back(charge_light_decoder_->GetSlotNumber());
    event_number_v_.push_back(charge_light_decoder_->GetEventNumber());
    num_adc_word_v_.push_back(charge_light_decoder_->GetNumAdcWords());
    event_frame_number_v_.push_back(charge_light_decoder_->GetEventFrameNumber());
    trigger_frame_number_v_.push_back(charge_light_decoder_->GetTriggerFrameNumber());
    check_sum_v_.push_back(charge_light_decoder_->GetCheckSum());
    trigger_sample_v_.push_back(charge_light_decoder_->GetTriggerSample());
}

void ProcessEvents::ClearFemVectors() {
    charge_light_decoder_->HeaderWord = 0;
    charge_channel_number_ = 0;
    charge_channel_.clear();
    charge_adc_.clear();
    light_channel_.clear();
    light_adc_.clear();
    light_frame_number_.clear();
    light_sample_number_.clear();
    event_number_v_.clear();
    slot_number_v_.clear();
    num_adc_word_v_.clear();
    event_frame_number_v_.clear();
    trigger_frame_number_v_.clear();
    check_sum_v_.clear();
    trigger_sample_v_.clear();
}

bool ProcessEvents::GetNumEvents(const size_t num_events) {
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

void ProcessEvents::FillFemDict() {

    // Clear and then init the vectors
    channel_full_waveform_.clear();
    channel_full_axis_.clear();
    for (size_t i = 0; i < num_light_channels_; i++) {
        channel_full_waveform_.emplace_back();
        channel_full_axis_.emplace_back();
    }

#ifdef USE_PYBIND11
    pybind11::dict fem_dict_;
    // FEM header
    fem_dict_["slot_number"] = vector_to_numpy_array_1d(slot_number_v_);
    fem_dict_["num_adc_word"] = vector_to_numpy_array_1d(num_adc_word_v_);
    fem_dict_["event_number"] = vector_to_numpy_array_1d(event_number_v_);
    fem_dict_["event_frame_number"] = vector_to_numpy_array_1d(event_frame_number_v_);
    fem_dict_["trigger_frame_number"] = vector_to_numpy_array_1d(trigger_frame_number_v_);
    fem_dict_["check_sum"] = vector_to_numpy_array_1d(check_sum_v_);
    fem_dict_["trigger_sample"] = vector_to_numpy_array_1d(trigger_sample_v_);
    // Light
    fem_dict_["light_channel"] = vector_to_numpy_array_1d(light_channel_);
    fem_dict_["light_frame_number"] = vector_to_numpy_array_1d(light_frame_number_);
    fem_dict_["light_readout_sample"] = vector_to_numpy_array_1d(light_sample_number_);
    fem_dict_["light_adc_words"] = vector_to_numpy_array_2d(light_adc_);
    // Charge
    fem_dict_["charge_channel"] = vector_to_numpy_array_1d(charge_channel_);
    fem_dict_["charge_adc_words"] = vector_to_numpy_array_2d(charge_adc_);

    event_dict_ = fem_dict_;

#else

    event_struct_.clear_event();
    event_struct_.slot_number = std::move(slot_number_v_);
    event_struct_.num_adc_word = std::move(num_adc_word_v_);
    event_struct_.event_number = std::move(event_number_v_);
    event_struct_.event_frame_number = std::move(event_frame_number_v_);
    event_struct_.trigger_frame_number = std::move(trigger_frame_number_v_);
    event_struct_.check_sum = std::move(check_sum_v_);
    event_struct_.trigger_sample = std::move(trigger_sample_v_);
    event_struct_.light_channel = std::move(light_channel_);
    event_struct_.light_frame_number = std::move(light_frame_number_);
    event_struct_.light_sample_number = std::move(light_sample_number_);
    event_struct_.light_adc = std::move(light_adc_);
    event_struct_.charge_channel = std::move(charge_channel_);
    event_struct_.charge_adc = std::move(charge_adc_);

#endif
}


// py::array_t<uint16_t> ExtReconstructLightWaveforms(uint16_t channel, py::array_t<uint16_t> &channels,
//     py::array_t<uint32_t> &samples, py::array_t<uint32_t> &frames, py::array_t<uint16_t> &adc_words, uint16_t time_size) {
//
//     constexpr int samples_per_frame = 255 * 32; // timesize * 32MHz
//     std::array<uint16_t, 4 * samples_per_frame> channel_full_waveform{};
//
//     for (auto &sample : channel_full_waveform) { sample = 2048; }
//
//     //######################
//     // Get buffer info
//     const py::buffer_info buf_ch = channels.request();
//     const py::buffer_info buf_sample = samples.request();
//     const py::buffer_info buf_frame = frames.request();
//     const py::buffer_info buf_adc_word = adc_words.request();
//
//     // Access data
//     auto* channel_ptr = static_cast<uint16_t*>(buf_ch.ptr);
//     auto* sample_ptr = static_cast<uint32_t*>(buf_sample.ptr);
//     auto* frame_ptr = static_cast<uint32_t*>(buf_frame.ptr);
//     auto* adc_word_ptr = static_cast<uint16_t*>(buf_adc_word.ptr);
//     //########################
//
//     const auto min_it = std::min_element(frame_ptr, frame_ptr + buf_frame.size);
//     uint32_t min_frame_number = min_it != (frame_ptr + buf_frame.size) ? *min_it : 0;
//
//     for (size_t roi = 0; roi < buf_ch.size; roi++) {
//         if (channel_ptr[roi] != channel) continue;
//         size_t frame_offset = (frame_ptr[roi] - min_frame_number) * samples_per_frame;
//         size_t start_idx = sample_ptr[roi] + frame_offset;
//         size_t end_idx = std::min((start_idx + buf_adc_word.shape[1]), channel_full_waveform.size());
//         for (size_t idx = start_idx; idx < end_idx; idx++) {
//             channel_full_waveform.at(idx) = adc_word_ptr[(roi*buf_adc_word.shape[1]) + (idx - start_idx)];
//         }
//     }
//     return to_numpy_array_1d(channel_full_waveform);
// }

// py::array_t<double> ExtReconstructLightAxis(uint32_t trig_frame, uint32_t trig_sample,
//     py::array_t<uint32_t> &frames, uint16_t time_size) {
//     constexpr int samples_per_frame = 255 * 32; // timesize * 32MHz
//     constexpr double light_sample_interval = 15.625;
//
//     //######################
//     // Get buffer info
//     const py::buffer_info buf_frame = frames.request();
//     auto* frame_ptr = static_cast<uint32_t*>(buf_frame.ptr);
//
//     const auto min_it = std::min_element(frame_ptr, frame_ptr + buf_frame.size);
//     uint16_t min_frame_number = min_it != (frame_ptr + buf_frame.size) ? *min_it : 0;
//
//     const double frame_offset = (trig_frame - min_frame_number) * light_sample_interval;
//     const double trigger_index = frame_offset + (trig_sample * 32);
//
//     std::array<double, 4 * samples_per_frame> light_axis{};
//     double tick_idx = 0;
//     for (auto &tick : light_axis) {
//         tick = (tick_idx - trigger_index) * light_sample_interval;
//         tick_idx++;
//     }
//     return py::array_t(light_axis.size(), &light_axis[0]);
// }

// py::array_t<double> ReconstructLightAxis() {
//     constexpr int samples_per_frame = 255 * 32; // timesize * 32MHz
//     constexpr double light_sample_interval = 15.625;
//
//     const auto min_it = std::min_element(light_frame_number_.begin(), light_frame_number_.end());
//     uint32_t min_frame_number = min_it != light_frame_number_.end() ? *min_it : 0;
//
//     const double frame_offset = (charge_light_decoder_->GetTriggerFrameNumber() - min_frame_number) * light_sample_interval;
//     const double trigger_index = frame_offset + (charge_light_decoder_->GetTriggerSample() * 32);
//
//     std::array<double, 10 * samples_per_frame> light_axis{};
//     double tick_idx = 0;
//     for (auto &tick : light_axis) {
//         tick = (tick_idx - trigger_index) * light_sample_interval;
//         tick_idx++;
//     }
//     return py::array_t(light_axis.size(), &light_axis[0]);
// }

// void ProcessEvents::ReconstructLightWaveforms() {
//     constexpr size_t samples_per_frame = 255 * 32; // timesize * 32MHz
//
//     const auto min_it = std::min_element(light_frame_number_.begin(), light_frame_number_.end());
//     uint32_t min_frame_number = min_it != light_frame_number_.end() ? *min_it : 0;
//
//     for (size_t roi = 0; roi < light_channel_.size(); roi++) {
//         size_t channel = light_channel_.at(roi);
//         if (channel > 31) continue;
//         size_t frame_offset = (light_frame_number_.at(roi) - min_frame_number) * samples_per_frame;
//         size_t start_idx = light_sample_number_.at(roi) + frame_offset;
//         size_t end_idx = std::min((start_idx + light_adc_.at(roi).size()), samples_per_frame*8);
//         for (size_t idx = start_idx; idx < end_idx; idx++) {
//             // channel_full_waveform.at(channel).at(idx) = light_adc_.at(roi).at(idx - start_idx);
//             channel_full_waveform_[channel].push_back(light_adc_.at(roi).at(idx - start_idx));
//             channel_full_axis_[channel].push_back(idx);
//         }
//     }
// }

