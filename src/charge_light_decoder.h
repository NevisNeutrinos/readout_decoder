//
// Created by Jon Sensenig on 3/11/25.
//

#ifndef CHARGE_LIGHT_DECODER_H
#define CHARGE_LIGHT_DECODER_H

#include <fstream>
#include <cstdint>
#include <iostream>

namespace decoder {

#pragma pack(push, 1) // Ensure no padding in structures

    // Define the structure for the 32-bit data words

    /* Define the structure for FEM Header
     *  There are 6 32b FEM headers which are the same for both the charge
     *  and light readout. The words should arrive in sequential order with
     *  the bits arranged such that 32b = [16b, 16b] where the rightmost 16b
     *  word arrives first, followed by the left 16b word, e.g. a stream is,
     *
     *  [16bword0_R, 16bword0_L, 16bword1_R, 16bword1_L, ...., 16bword0_N, 16bwordN_L]
     *
     *  This is the case for all words so the structures assume this order.
     */

    // header 1
    struct FEMHeader1 {
        const uint16_t event_start : 16;
        const uint16_t slot_number : 5; // First 5 bits
        const uint16_t fem_id : 4;
        const uint16_t test : 1;
        const uint16_t overflow : 1;
        const uint16_t full : 1;
        const uint16_t header_start_1 : 4;
    };

    // header 2
    struct FEMHeader2 {
        const uint16_t num_adc_words_upper : 12;
        const uint16_t header_pack_0 : 4;
        const uint16_t num_adc_words_lower : 12;
        const uint16_t header_pack_1 : 4;

        u_int32_t num_adc_words() const { return ((num_adc_words_upper << 12) & 0xFFF000) | (num_adc_words_lower & 0xFFF); }
    };

    // header 3
    struct FEMHeader3 {
        const uint16_t event_number_upper : 12;
        const uint16_t header_pack_2 : 4;
        const uint16_t event_number_lower : 12;
        const uint16_t header_pack_3 : 4;

        u_int32_t event_number() const { return ((event_number_upper << 12) & 0xFFF000) | (event_number_lower & 0xFFF); }
    };

    // header 4 FIXME this should be trigger frame
     struct FEMHeader4 {
        const uint16_t frame_number_upper : 12;
        const uint16_t header_pack_4 : 4;
        const uint16_t frame_number_lower : 12;
        const uint16_t header_pack_5 : 4;

        u_int32_t event_frame_number() const { return ((frame_number_upper << 12) & 0xFFF000) | (frame_number_lower & 0xFFF); }
    };

    // header 5
    struct FEMHeader5 {
        const uint16_t checksum_upper : 12;
        const uint16_t header_pack_6 : 4;
        const uint16_t checksum_lower : 12;
        const uint16_t header_pack_7 : 4;

        u_int32_t checksum() const { return ((checksum_upper << 12) & 0xFFF000) | (checksum_lower & 0xFFF); }
    };

    // header 6
    struct FEMHeader6 {
        const uint16_t trig_sample_number_upper : 4;
        const uint16_t trig_frame_number_lower : 4;
        const uint16_t pad0 : 4;
        const uint16_t header_pack_8 : 4;
        const uint16_t trig_sample_number_lower : 8;
        const uint16_t pad1 : 4;
        const uint16_t header_pack_9 : 4;

        u_int32_t trig_sample_number() const { return ((trig_sample_number_upper << 4) & 0xF00) | (trig_sample_number_lower & 0xFF); }
        u_int32_t trig_frame_number() const { return (trig_frame_number_lower & 0xF); }
    };


    // Define the structure for FEM Header
    struct LightHeader1 {
        const uint16_t channel : 6;
        const uint16_t pad0 : 3;
        const uint16_t id : 3;
        const uint16_t header_tag : 2;
        const uint16_t word_tag: 2;
    };

    struct LightHeader2 {
        const uint16_t sample_num_upper : 5;
        const uint16_t frame_num : 3;
        const uint16_t pad0 : 8;
    };

    struct LightHeader3 {
        const uint16_t sample_num_lower : 12;
        const uint16_t pad0 : 4;
    };

    // General form for the ADC words, both charge and light
    struct AdcWord {
        const uint16_t pad0 : 4;
        const uint16_t adc_word1 : 12;

        uint16_t get_adc1() const { return adc_word1; }
    };

#pragma pack(pop) // Restore default alignment

    class Decoder {

    public:

        Decoder() = default;
        ~Decoder() = default;

        static constexpr uint32_t event_start_ = 0xFFFFFFFF;
        static constexpr uint32_t event_end_ = 0xE0000000;
        static constexpr uint16_t header_word_ = 0xF000;
        static constexpr uint16_t charge_channel_start_ = 0x4000;
        static constexpr uint16_t charge_channel_end_ = 0x5000;
        static constexpr uint16_t light_channel_start_ = 0x4000;
        static constexpr uint16_t light_channel_end_ = 0xC000;
        static constexpr uint16_t light_channel_intmed_ = 0x8000;
        static constexpr uint16_t light_roi_header1_ = 0x1000;
        static constexpr uint16_t light_roi_header2_ = 0x2000;
        static constexpr uint16_t light_roi_end_ = 0x3000;

        FEMHeader1 fem_header1_t{};
        FEMHeader2 fem_header2_t{};
        FEMHeader3 fem_header3_t{};
        FEMHeader4 fem_header4_t{};
        FEMHeader5 fem_header5_t{};
        FEMHeader6 fem_header6_t{};
        AdcWord adc_word_t{};
        LightHeader1 light_header1_t{};
        LightHeader2 light_header2_t{};
        LightHeader3 light_header3_t{};

        static bool IsEventStart(const uint32_t word) {return (word & 0xFFFFFFFF) == event_start_;}
        static bool IsEventEnd(const uint32_t word) {return (word & 0xFFFFFFFF) == event_end_;}
        static bool IsHeaderWord(const uint16_t word) {return (word & 0xF000) == header_word_;}
        static bool ChargeChannelStart(const uint16_t word) {return (word & 0xF000) == charge_channel_start_;}
        static bool ChargeChannelEnd(const uint16_t word) {return (word & 0xF000) == charge_channel_end_;}
        static bool LightChannelStart(const uint16_t word) {return (word & 0xC000) == light_channel_start_;}
        static bool LightChannelEnd(const uint16_t word) {return (word & 0xC000) == light_channel_end_;}
        static bool LightChannelIntmed(const uint16_t word) {return (word & 0xC000) == light_channel_intmed_;}
        static bool LightRoiHeader1(const uint16_t word) {return (word & 0x3000) == light_roi_header1_;}
        static bool LightRoiHeader2(const uint16_t word) {return (word & 0x3000) == light_roi_header2_;}
        static bool LightRoiEnd(const uint16_t word) {return (word & 0x3000) == light_roi_end_;}

        void FemHeaderDecode(uint32_t header_word);
        bool FemLightDecode(uint16_t header_word);
        void DecodeAdcWord(uint16_t word);

        uint16_t GetSlotNumber() const { return fem_header1_t.slot_number; }
        uint16_t GetLightFrameNumber() const;
        uint16_t GetTriggerFrameNumber() const;
        std::array<uint16_t, 1024> GetAdcWords () { adc_count_ = 0; return adc_word_array_; }
        uint16_t GetNumberOfAdcWords() const { return fem_header2_t.num_adc_words(); }

        int HeaderWord{};
        int LightWord{};

        static constexpr int kChargeAdcWord = 0;
        static constexpr int kLightAdcWord = 1;

    private:

        static uint16_t CorrectRollover(const uint16_t word1, const uint16_t word2) {
            if ((word1 - word2) > 4) return word1 - 8;
            if ((word1 - word2) < -4) return word1 + 8;
            return word1;
        }

        size_t adc_count_ = 0;
        std::array<uint16_t, 1024> adc_word_array_{};

    };

}

#endif //CHARGE_LIGHT_DECODER_H
