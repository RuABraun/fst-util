cmake_minimum_required(VERSION 3.9.5)
SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS "-Ofast ")

project(condutor)

include_directories("/remote/idiap.svm/temp.speech01/rbraun/code/kaldi/tools/openfst-1.6.7/include")
include_directories("lib/")

add_subdirectory(lib/pybind11)
pybind11_add_module(wrappedfst lib/fst-wrapper.cc)

set_target_properties(wrappedfst PROPERTIES LIBRARY_OUTPUT_NAME "wrappedfst")

target_link_libraries(wrappedfst PRIVATE "-L/remote/idiap.svm/temp.speech01/rbraun/code/kaldi/tools/openfst-1.6.7/lib" -lfstscript -lfst)
