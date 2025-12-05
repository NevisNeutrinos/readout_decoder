#include <iostream>
#include "src/process_events.h"
// #include <pybind11/embed.h>

int main() {

    // Make sure the python interpreter is initialized
    // py::scoped_interpreter guard;

    const uint16_t light_slot = 16;
    const bool use_charge_roi = false;
    const std::vector<uint16_t> channel_threshold(64, 0);
    const bool skip_beam_roi = false;
    ProcessEvents events(light_slot, use_charge_roi, channel_threshold, skip_beam_roi);

    // const std::string filename = "/Users/jonsensenig/work/grams/daq_ana/jon_test/pGRAMS_bin_992.dat";
    // const std::string filename = "/home/pgrams/data/nov2025_integration_data/pGRAMS_bin_435_0.dat";
    const std::string filename = "/home/pgrams/data/sabertooth2_data/data/readout_data/pGRAMS_bin_196_0.dat";
    events.OpenFile(filename);
    events.GetNumEvents(5);

    // for (size_t i = 0; i < 10; i++) { std::cout << i << " "; events.GetEvent(); }

    return 0;
}
