//
// Created by Jon Sensenig on 3/14/25.
//

#include "process_events.h"
#include "process_events_py.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>


namespace py = pybind11;

py::array_t<uint16_t> ExtReconstructLightWaveforms(uint16_t channel, py::array_t<uint16_t> &channels,
    py::array_t<uint32_t> &samples, py::array_t<uint32_t> &frames, py::array_t<uint16_t> &adc_words, uint16_t time_size) {

    constexpr int samples_per_frame = 255 * 32; // timesize * 32MHz
    std::array<uint16_t, 4 * samples_per_frame> channel_full_waveform{};

    for (auto &sample : channel_full_waveform) { sample = 2048; }

    //######################
    // Get buffer info
    const py::buffer_info buf_ch = channels.request();
    const py::buffer_info buf_sample = samples.request();
    const py::buffer_info buf_frame = frames.request();
    const py::buffer_info buf_adc_word = adc_words.request();

    // Access data
    auto* channel_ptr = static_cast<uint16_t*>(buf_ch.ptr);
    auto* sample_ptr = static_cast<uint32_t*>(buf_sample.ptr);
    auto* frame_ptr = static_cast<uint32_t*>(buf_frame.ptr);
    auto* adc_word_ptr = static_cast<uint16_t*>(buf_adc_word.ptr);
    //########################

    const auto min_it = std::min_element(frame_ptr, frame_ptr + buf_frame.size);
    uint32_t min_frame_number = min_it != (frame_ptr + buf_frame.size) ? *min_it : 0;

    for (size_t roi = 0; roi < buf_ch.size; roi++) {
        if (channel_ptr[roi] != channel) continue;
        size_t frame_offset = (frame_ptr[roi] - min_frame_number) * samples_per_frame;
        size_t start_idx = sample_ptr[roi] + frame_offset;
        size_t end_idx = std::min((start_idx + buf_adc_word.shape[1]), channel_full_waveform.size());
        for (size_t idx = start_idx; idx < end_idx; idx++) {
            channel_full_waveform.at(idx) = adc_word_ptr[(roi*buf_adc_word.shape[1]) + (idx - start_idx)];
        }
    }
    return to_numpy_array_1d(channel_full_waveform);
}

py::array_t<double> ExtReconstructLightAxis(uint32_t trig_frame, uint32_t trig_sample,
    py::array_t<uint32_t> &frames, uint16_t time_size) {
    constexpr int samples_per_frame = 255 * 32; // timesize * 32MHz
    constexpr double light_sample_interval = 15.625;

    //######################
    // Get buffer info
    const py::buffer_info buf_frame = frames.request();
    auto* frame_ptr = static_cast<uint32_t*>(buf_frame.ptr);

    const auto min_it = std::min_element(frame_ptr, frame_ptr + buf_frame.size);
    uint16_t min_frame_number = min_it != (frame_ptr + buf_frame.size) ? *min_it : 0;

    const double frame_offset = (trig_frame - min_frame_number) * light_sample_interval;
    const double trigger_index = frame_offset + (trig_sample * 32);

    std::array<double, 4 * samples_per_frame> light_axis{};
    double tick_idx = 0;
    for (auto &tick : light_axis) {
        tick = (tick_idx - trigger_index) * light_sample_interval;
        tick_idx++;
    }
    return py::array_t(light_axis.size(), &light_axis[0]);
}

PYBIND11_MODULE(decoder_bindings, m) {
    py::class_<ProcessEvents>(m, "ProcessEvents")
        .def(py::init<const uint16_t>(), py::arg("light_slot")) // Constructor
        .def("open_file", &ProcessEvents::OpenFile, py::arg("filename"))
        .def("get_event", &ProcessEvents::GetEvent)
        .def("get_num_events", &ProcessEvents::GetNumEvents, py::arg("num_events"))
        .def("get_event_dict", &ProcessEvents::GetEventDict);
        m.def("get_full_light_waveform", &ExtReconstructLightWaveforms);
        m.def("get_full_light_axis", &ExtReconstructLightAxis);
}
