cmake_minimum_required(VERSION 3.30)
project(raw_decoder)

set(CMAKE_CXX_STANDARD 17)

include_directories(src
                extern/pybind11)

add_subdirectory(extern/pybind11)
add_executable(raw_decoder run_decode.cpp
        src/charge_light_decoder.h
        src/charge_light_decoder.cpp
        src/process_events.cpp
        src/process_events.h)

target_link_libraries(raw_decoder PRIVATE pybind11::module pybind11::embed )