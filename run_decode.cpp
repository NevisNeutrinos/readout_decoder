#include <iostream>
#include <pybind11/embed.h>
#include "src/process_events.h"


int main() {

    // Make sure the python interpreter is initialized
    py::scoped_interpreter guard;

    ProcessEvents events;

    const std::string filename = "/Users/jonsensenig/work/grams/daq_ana/jon_test/pGRAMS_bin_992.dat";
    events.OpenFile(filename);
    events.GetNumEvents(200);

    // for (size_t i = 0; i < 10; i++) { std::cout << i << " "; events.GetEvent(); }

    return 0;
}
