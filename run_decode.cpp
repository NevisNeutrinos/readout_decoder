#include <iostream>
#include "src/process_events.h"


int main() {
    ProcessEvents events;

    const std::string filename = "/Users/jonsensenig/work/grams/daq_ana/jon_test/pGRAMS_bin_992.dat";
    events.OpenFile(filename);
    events.GetNumEvents(10);
    return 0;
}
