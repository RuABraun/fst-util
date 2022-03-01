cmake_minimum_required(VERSION 3.9.5)
SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS "-O3 ")

project(fst-util)

include_directories("/home/rudolf/code/openfst-1.6.9/install/include")
include_directories("/home/rudolf/code/openfst-1.6.9/")
include_directories("lib/")

add_subdirectory(lib/pybind11)
pybind11_add_module(wrappedfst lib/fst-wrapper.cc)

set_target_properties(wrappedfst PROPERTIES LIBRARY_OUTPUT_NAME "wrappedfst")

target_link_libraries(wrappedfst PRIVATE "-L/home/rudolf/code/openfst-1.6.9/install/lib" -lfstscript -lfst )
