#include <iostream>
#include "src/process_events.h"
#include <pybind11/embed.h>

int main() {

    // Make sure the python interpreter is initialized
    py::scoped_interpreter guard;

    const uint16_t light_slot = 16;
    ProcessEvents events(light_slot);

    // const std::string filename = "/Users/jonsensenig/work/grams/daq_ana/jon_test/pGRAMS_bin_992.dat";
    const std::string filename = "/Users/jonsensenig/work/grams/daq_ana/jon_test/pGRAMS_bin_1.dat";
    events.OpenFile(filename);
    events.GetNumEvents(5);

    // for (size_t i = 0; i < 10; i++) { std::cout << i << " "; events.GetEvent(); }

    return 0;
}
