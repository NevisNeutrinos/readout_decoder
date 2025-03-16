cmake_minimum_required(VERSION 3.30)
project(decoder_bindings)

set(CMAKE_CXX_STANDARD 17)

find_package(pybind11 CONFIG REQUIRED)
#add_subdirectory(extern/pybind11)
include_directories(../src)
pybind11_add_module(decoder_bindings
        src/decoder_bindings.cpp
        ../src/process_events.cpp
        ../src/charge_light_decoder.cpp)

install(TARGETS decoder_bindings DESTINATION .)