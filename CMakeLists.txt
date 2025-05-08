cmake_minimum_required(VERSION 3.30)
project(raw_decoder)

set(CMAKE_CXX_STANDARD 17)
set(USE_PYTHON TRUE)
message("SET PYTHON TO ${USE_PYTHON}")

file(GLOB DECODER_SRC src/*cpp)
if(USE_PYTHON)
    message("Compiling decoder with python..")
    add_compile_definitions(USE_PYBIND11=1)
    include_directories(src extern/pybind11)
    add_subdirectory(extern/pybind11)
    add_executable(raw_decoder run_decode.cpp
                ${DECODER_SRC})
    target_link_libraries(raw_decoder PRIVATE pybind11::module pybind11::embed)
else()
    message("Compiling decoder without python..")
    include_directories(src)
    add_library(raw_decoder STATIC src/process_events.cpp
                                    src/charge_light_decoder.cpp)
    INSTALL(TARGETS raw_decoder DESTINATION .)
    message("Installed raw_decoder!")
endif ()
