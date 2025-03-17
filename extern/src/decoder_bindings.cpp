//
// Created by Jon Sensenig on 3/14/25.
//

#include <process_events.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>


namespace py = pybind11;

PYBIND11_MODULE(decoder_bindings, m) {
    py::class_<ProcessEvents>(m, "ProcessEvents")
        .def(py::init<>()) // Constructor
        .def("open_file", &ProcessEvents::OpenFile)
        .def("get_event", &ProcessEvents::GetEvent)
        .def("get_num_events", &ProcessEvents::GetNumEvents)
        .def("get_event_dict", &ProcessEvents::GetEventDict)
        .def("get_full_light_waveform", &ProcessEvents::ExtReconstructLightWaveforms)
        .def("get_full_light_axis", &ProcessEvents::ExtReconstructLightAxis);
}