//
// Created by Jon Sensenig on 3/12/25.
//
#include "charge_light_decoder.h"

namespace decoder {

    void Decoder::DecodeAdcWord(const uint16_t word) {
        std::memcpy(&adc_word_t, &word, sizeof(adc_word_t));
        adc_word_array_.push_back(adc_word_t.adc_word1);
    }

    bool Decoder::FemLightDecode(const uint16_t header_word) {
        // There are 3 header words, step through each fo them and get their info
        switch (LightWord) {
            case 0: { // header 1
                std::memcpy(&light_header1_t, &header_word, sizeof(light_header1_t));
                LightWord++;
                return false;
            }
            case 1: { // header 2
                std::memcpy(&light_header2_t, &header_word, sizeof(light_header2_t));
                LightWord++;
                return false;
            }
            case 2: { // header 3
                std::memcpy(&light_header3_t, &header_word, sizeof(light_header3_t));
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
                HeaderWord++;
                break;
            }
            case 1: { // header 2
                std::memcpy(&fem_header2_t, &header_word, sizeof(fem_header2_t));
                HeaderWord++;
                break;
            }
            case 2: { // header 3
                std::memcpy(&fem_header3_t, &header_word, sizeof(fem_header3_t));
                HeaderWord++;
                break;
            }
            case 3: { // header 4
                std::memcpy(&fem_header4_t, &header_word, sizeof(fem_header4_t));
                HeaderWord++;
                break;
            }
            case 4: { // header 5
                std::memcpy(&fem_header5_t, &header_word, sizeof(fem_header5_t));
                HeaderWord++;
                break;
            }
            case 5: { // header 6
                std::memcpy(&fem_header6_t, &header_word, sizeof(fem_header6_t));
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