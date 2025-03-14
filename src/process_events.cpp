//
// Created by Jon Sensenig on 3/14/25.
//

#include "process_events.h"
#include "charge_light_decoder.h"
#include <cerrno>

ProcessEvents::ProcessEvents(): charge_light_decoder_(nullptr) {
    charge_light_decoder_ = new decoder::Decoder;
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
    size_t event_start_count = 0;
    size_t event_end_count = 0;
    bool read_charge_channel = false;
    bool read_light_channel = false;
    bool light_word_header_done = false;
    size_t light_channel_roi_header = 0;
    size_t light_channel_roi = 0;

    // This will run from the start of the event until
    // end of event marker is reached or if all words are
    // read from file.

    // for (size_t word_idx = 0; word_idx < file_num_words_; word_idx++) {
    while (word_idx_ < file_num_words_) {
        const uint32_t word_32 = file_buffer_[word_idx_];
        word_idx_++;
        if (decoder::Decoder::IsEventStart(word_32)) {
            // Reset the FEM header decoder state machine
            charge_light_decoder_->HeaderWord = 0;
            event_start_count++;
            continue;
        } if (decoder::Decoder::IsEventEnd(word_32)) {
            event_end_count++;
            return false;
        }

        if (decoder::Decoder::IsHeaderWord(word_32)) {
            charge_light_decoder_->FemHeaderDecode(word_32);
            continue;
        }

        const uint16_t slot_number = charge_light_decoder_->GetSlotNumber();
        for (size_t j = 0; j < 2; j++) {

            // FIXME The 16b words should be aligned as a 32b word at this point but should add check
            // 32b word & 0xFFFF is 1R the 1st word
            // (32b word >> 16) & 0xFFFF is 1L the 2nd word
            uint16_t word = j == 0 ? word_32 & 0xFFFF : (word_32 >> 16) & 0xFFFF;

            if (decoder::Decoder::ChargeChannelStart(word) && slot_number == 15) {
                // std::cout << "ChargeChannel Start " << (word & 0x3F) << "\n";
                read_charge_channel = true;
            }
            else if (decoder::Decoder::ChargeChannelEnd(word) && slot_number == 15) {
                charge_light_decoder_->GetAdcWords();
                read_charge_channel = false;
            }
            else if (read_charge_channel) {
                charge_light_decoder_->DecodeAdcWord(word);
            }
            else if (decoder::Decoder::LightChannelStart(word) && slot_number == 16) {
                std::cout << "LightChannel Start \n";
                read_light_channel = true;
            }
            else if (decoder::Decoder::LightChannelEnd(word) && slot_number == 16) {
                std::cout << "LightChannel End \n";
                read_light_channel = false;
            }
            else if (read_light_channel) {
                if (!decoder::Decoder::LightChannelIntmed(word)) {
                    std::cerr << "Unexpected word ID!" << std::endl;
                }
                if (decoder::Decoder::LightRoiHeader1(word) || !light_word_header_done) {
                    light_channel_roi_header++;
                    light_word_header_done = charge_light_decoder_->FemLightDecode(word);
                }
                else if (decoder::Decoder::LightRoiHeader2(word)) {
                    charge_light_decoder_->DecodeAdcWord(word);
                }
                else if (decoder::Decoder::LightRoiEnd(word)) {
                    charge_light_decoder_->LightWord = 0;
                    charge_light_decoder_->GetAdcWords();
                    light_word_header_done = false;
                    light_channel_roi++;
                }
                else {
                    std::cout << "Unexpected light word! " << (word & 0x3000)  << " "
                    << light_word_header_done << std::endl;
                }
            }
        }
    }

    if (data_file_ != nullptr) {
        fclose(data_file_);
        data_file_ = nullptr;
    }

    std::cout << "event_start_count: " << event_start_count << std::endl;
    std::cout << "event_end_count: " << event_end_count << std::endl;
    std::cout << "Light ROIs Found x3 (3 headers): " << light_channel_roi_header << std::endl;
    std::cout << "Light ROIs Found: " << light_channel_roi << std::endl;
    return true;
}

bool ProcessEvents::GetNumEvents(int num_events) {

    size_t event_count = 0;
    while (!GetEvent() && num_events > 0) {
        event_count++;
    }

    if (data_file_ != nullptr) {
        fclose(data_file_);
        data_file_ = nullptr;
    }
    return true;
}