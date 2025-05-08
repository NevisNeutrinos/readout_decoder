//
// Created by Jon Sensenig on 3/14/25.
//

#ifndef PROCESS_EVENTS_H
#define PROCESS_EVENTS_H

#include "charge_light_decoder.h"
#include <string>
#include <iostream>
#include <memory>

#ifdef USE_PYBIND11
    #include "process_events_py.h"
#endif

struct EventStruct {
    // Charge
    std::vector<uint16_t> charge_channel;
    std::vector<std::vector<uint16_t>> charge_adc;
    // Light
    std::vector<uint16_t> light_channel;
    std::vector<uint32_t> light_frame_number;
    std::vector<uint16_t> light_sample_number; // 32b
    std::vector<std::vector<uint16_t>> light_adc;
    // FEM data
    std::vector<uint16_t> slot_number;
    std::vector<uint32_t> num_adc_word;
    std::vector<uint32_t> event_number; //32b
    std::vector<uint32_t> event_frame_number; //32b
    std::vector<uint32_t> trigger_frame_number; //32b
    std::vector<uint32_t> check_sum; //32b
    std::vector<uint32_t> trigger_sample; //32b

    void clear_event() {
        // Charge
        charge_channel.clear();
        charge_adc.clear();
        // Light
        light_channel.clear();
        light_frame_number.clear();
        light_sample_number.clear(); // 32b
        light_adc.clear();
        // FEM data
        slot_number.clear();
        num_adc_word.clear();
        event_number.clear(); //32b
        event_frame_number.clear(); //32b
        trigger_frame_number.clear(); //32b
        check_sum.clear(); //32b
        trigger_sample.clear(); //32b
    }
};

class ProcessEvents {
public:
    explicit ProcessEvents(uint16_t light_slot);
    ~ProcessEvents();

    bool OpenFile(const std::string &file_name);
    bool GetNumEvents(size_t num_events);
    bool GetEvent();

    void FillFemDict();
    void SetFemData();
    void ClearFemVectors();
    void ReconstructLightWaveforms();

#ifdef USE_PYBIND11
    // For each FEM fill a python dictionary
    py::dict event_dict_;

    pybind11::dict GetEventDict() { return event_dict_; };
    pybind11::array_t<double> ReconstructLightAxis();
#endif

private:

    static constexpr size_t num_light_channels_ = 32;

    std::unique_ptr<decoder::Decoder> charge_light_decoder_;
    FILE *data_file_{};
    std::unique_ptr<int[]> file_buffer_{};

    size_t file_num_words_{};
    size_t word_idx_ = 0;

    // Charge ADC arrays
    size_t event_number_ = 0;
    size_t charge_channel_number_ = 0;
    size_t light_roi_number_ = 0;
    uint16_t light_slot_ = 0;

    std::array<std::array<uint16_t, 595>, 64> charge_adc_arr_{};
    std::vector<std::vector<uint16_t>> charge_adc_{};
    std::vector<std::vector<uint16_t>> light_adc_{};
    std::vector<uint16_t> charge_channel_{};
    std::vector<uint16_t> light_channel_{};
    std::vector<uint32_t> light_frame_number_{};
    std::vector<uint16_t> light_sample_number_{}; // 32b
    std::vector<std::vector<uint16_t>> channel_full_waveform_{};
    std::vector<std::vector<size_t>> channel_full_axis_{};

    // FEM data
    std::vector<uint16_t> slot_number_v_;
    std::vector<uint32_t> num_adc_word_v_;
    std::vector<uint32_t> event_number_v_; //32b
    std::vector<uint32_t> event_frame_number_v_; //32b
    std::vector<uint32_t> trigger_frame_number_v_; //32b
    std::vector<uint32_t> check_sum_v_; //32b
    std::vector<uint32_t> trigger_sample_v_; //32b

    // The event struct for when using within C++
    EventStruct event_struct_{};

};



#endif //PROCESS_EVENTS_H
