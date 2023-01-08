cmake_minimum_required(VERSION 3.25.1)

project(training_data_loader)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

add_library(training_data_loader SHARED training_data_loader.cpp)

install(
    TARGETS training_data_loader
    RUNTIME DESTINATION .
    LIBRARY DESTINATION .)