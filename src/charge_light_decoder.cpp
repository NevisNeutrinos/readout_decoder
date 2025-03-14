//
// Created by Jon Sensenig on 3/12/25.
//
#include "charge_light_decoder.h"

namespace decoder {

    void Decoder::DecodeAdcWord(const uint16_t word) {
        std::memcpy(&adc_word_t, &word, sizeof(adc_word_t));
        adc_word_array_.at(adc_count_) = adc_word_t.adc_word1;
        adc_count_++;
    }

    bool Decoder::FemLightDecode(const uint16_t header_word) {
        // There are 3 header words, step through each fo them and get their info
        switch (LightWord) {
            case 0: { // header 1
                std::memcpy(&light_header1_t, &header_word, sizeof(light_header1_t));
                std::cout << "Ch: " << light_header1_t.channel << std::endl;
                LightWord++;
                return false;
            }
            case 1: { // header 2
                std::memcpy(&light_header2_t, &header_word, sizeof(light_header2_t));
                // std::cout << "FN: " << light_header2_t.frame_num << std::endl;
                LightWord++;
                return false;
            }
            case 2: { // header 3
                std::memcpy(&light_header3_t, &header_word, sizeof(light_header3_t));
                // std::cout << "SN: " << light_header3_t.sample_num_lower << std::endl;
                LightWord = 0;
                return true;
            }
            default: {
                LightWord = 0;
                std::cerr << "FemLightDecode: Unknown LightWord " << LightWord << std::endl;
                return false;
            }
        }
    }

    void Decoder::FemHeaderDecode(const uint32_t header_word) {
        // There are 6 FEM header words, here we iterate through them
        switch (HeaderWord) {
            case 0: { // header 1
                std::memcpy(&fem_header1_t, &header_word, sizeof(fem_header1_t));
                std::cout << "Slot: " << fem_header1_t.slot_number << std::endl;
                HeaderWord++;
                break;
            }
            case 1: { // header 2
                std::memcpy(&fem_header2_t, &header_word, sizeof(fem_header2_t));
                std::cout << "nADC: " << fem_header2_t.num_adc_words() << std::endl;
                HeaderWord++;
                break;
            }
            case 2: { // header 3
                std::memcpy(&fem_header3_t, &header_word, sizeof(fem_header3_t));
                std::cout << "Evt No.: " << fem_header3_t.event_number() << std::endl;
                HeaderWord++;
                break;
            }
            case 3: { // header 4
                std::memcpy(&fem_header4_t, &header_word, sizeof(fem_header4_t));
                std::cout << "Evt Frame: " << fem_header4_t.event_frame_number() << std::endl;
                HeaderWord++;
                break;
            }
            case 4: { // header 5
                std::memcpy(&fem_header5_t, &header_word, sizeof(fem_header5_t));
                std::cout << "CheckSum: " << fem_header5_t.checksum() << std::endl;
                HeaderWord++;
                break;
            }
            case 5: { // header 6
                std::memcpy(&fem_header6_t, &header_word, sizeof(fem_header6_t));
                std::cout << "Trig Sample no.: " << fem_header6_t.trig_sample_number() << std::endl;
                HeaderWord = 0; // reset back to first wod
                break;
            }
            default: {
                std::cerr << "Unknown Header Word: " << HeaderWord << std::endl;
                HeaderWord = 0;
            }
        }
    }

    uint16_t Decoder::GetLightFrameNumber() const {
        const uint16_t fem_trigger_frame_number = fem_header6_t.trig_frame_number();
        const uint16_t frame_num = (fem_trigger_frame_number & 0xFFFFF8) | light_header2_t.frame_num;

        return CorrectRollover(frame_num, fem_header4_t.event_frame_number());
    }

    uint16_t Decoder::GetTriggerFrameNumber() const {
        const uint16_t fem_trigger_frame_number = fem_header6_t.trig_frame_number();
        const uint16_t trig_frame_num = (fem_trigger_frame_number & 0xFFFFF0) | fem_header6_t.trig_frame_number_lower;

        return CorrectRollover(fem_header4_t.event_frame_number(), trig_frame_num);
    }

} // decoder namespace