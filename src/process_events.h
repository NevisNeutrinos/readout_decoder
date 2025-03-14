//
// Created by Jon Sensenig on 3/14/25.
//

#ifndef PROCESS_EVENTS_H
#define PROCESS_EVENTS_H

#include <string>
#include <iostream>
#include "charge_light_decoder.h"

class ProcessEvents {
public:
    ProcessEvents();
    ~ProcessEvents();

    bool OpenFile(const std::string &file_name);
    bool GetNumEvents(int num_events);
    bool GetEvent();


private:

    decoder::Decoder *charge_light_decoder_;
    FILE *data_file_{};
    int *file_buffer_{};
    size_t file_num_words_{};
    size_t word_idx_ = 0;

};



#endif //PROCESS_EVENTS_H
