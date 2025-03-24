//
// Created by Jon Sensenig on 3/14/25.
//

#ifndef PROCESS_EVENTS_H
#define PROCESS_EVENTS_H

#include <string>
#include <iostream>
#include "charge_light_decoder.h"
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
// #include <pybind11/stl.h>

namespace py = pybind11;

class ProcessEvents {
public:
    ProcessEvents();
    ~ProcessEvents();

    bool OpenFile(const std::string &file_name);
    bool GetNumEvents(int num_events);
    bool GetEvent();

    void FillFemDict();
    void SetFemData();
    void ClearFemVectors();
    pybind11::dict GetEventDict() { return event_dict_; };
    void ReconstructLightWaveforms();
    py::array_t<double> ReconstructLightAxis();
    py::array_t<uint16_t> ExtReconstructLightWaveforms(uint16_t channel, py::array_t<uint16_t> &channels,
                        py::array_t<uint32_t> &samples, py::array_t<uint32_t> &frames, py::array_t<uint16_t> &adc_words);
    py::array_t<double> ExtReconstructLightAxis(uint32_t trig_frame, uint32_t trig_sample, py::array_t<uint32_t> &frames);

private:

    // For each FEM fill a python dictionary
    pybind11::dict event_dict_;
    static constexpr size_t num_light_channels_ = 32;

    // Convert 1D,2D std::array and std::vector to a NumPy array
    template <typename T>
    static py::array_t<T> vector_to_numpy_array_1d(const std::vector<T>& vec) {
        return py::array_t(vec.size(), vec.data());
    }

    // template <typename T>
    static py::array_t<uint16_t> vector_to_numpy_array_2d(const std::vector<std::vector<uint16_t>>& vec) {
        if (vec.empty()) {
            return py::array_t<uint16_t>({0, 0});  // Return empty array if input is empty
        }

        size_t rows = vec.size();
        size_t cols = vec.back().size();

        // Flatten 2D vector into 1D buffer
        std::vector<uint16_t> flat_data;
        flat_data.reserve(rows * cols);
        for (const auto& row : vec) {
            flat_data.insert(flat_data.end(), row.begin(), row.end());
        }

        // Create NumPy array with the correct shape, sharing the memory with `flat_data`
        return py::array_t({rows, cols}, flat_data.data());
    }

    template <size_t M>
    py::array_t<uint16_t> to_numpy_array_1d(const std::array<uint16_t, M>& arr) {
        return py::array_t<uint16_t>({M}, &arr[0]);
    }

    template <size_t M, size_t N>
    py::array_t<uint16_t> to_numpy_array_2d(const std::array<std::array<uint16_t, N>, M>& arr) {
        return py::array_t<uint16_t>({M, N}, &arr[0][0]);
    }

    decoder::Decoder *charge_light_decoder_;
    FILE *data_file_{};
    int *file_buffer_{};
    size_t file_num_words_{};
    size_t word_idx_ = 0;

    // Charge ADC arrays
    size_t event_number_ = 0;
    size_t charge_channel_number_ = 0;
    size_t light_roi_number_ = 0;

    std::array<std::array<uint16_t, 595>, 64> charge_adc_arr_{};
    std::vector<std::vector<uint16_t>> charge_adc_{};
    std::vector<std::vector<uint16_t>> light_adc_{};
    std::vector<uint16_t> charge_channel_{};
    std::vector<uint16_t> light_channel_{};
    std::vector<uint32_t> light_frame_number_{};
    std::vector<uint16_t> light_sample_number_{};
    std::vector<std::vector<uint16_t>> channel_full_waveform_{};
    std::vector<std::vector<size_t>> channel_full_axis_{};

    // FEM data
    std::vector<uint16_t> slot_number_v_;
    std::vector<uint32_t> num_adc_word_v_;
    std::vector<uint16_t> event_number_v_;
    std::vector<uint16_t> event_frame_number_v_;
    std::vector<uint16_t> trigger_frame_number_v_;
    std::vector<uint16_t> check_sum_v_;
    std::vector<uint16_t> trigger_sample_v_;


};



#endif //PROCESS_EVENTS_H
