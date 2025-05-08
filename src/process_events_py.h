//
// Created by Jon Sensenig on 5/7/25.
//

#ifndef PROCESS_EVENTS_PY_H
#define PROCESS_EVENTS_PY_H

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

    // For each FEM fill a python dictionary
    py::dict event_dict_;
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

    pybind11::dict GetEventDict() { return event_dict_; };
    py::array_t<double> ReconstructLightAxis();
//    py::array_t<uint16_t> ExtReconstructLightWaveforms(uint16_t channel, py::array_t<uint16_t> &channels,
//    py::array_t<uint32_t> &samples, py::array_t<uint32_t> &frames, py::array_t<uint16_t> &adc_words, uint16_t time_size);
//    py::array_t<double> ExtReconstructLightAxis(uint32_t trig_frame, uint32_t trig_sample, py::array_t<uint32_t> &frames, uint16_t time_size);

#endif //PROCESS_EVENTS_PY_H
