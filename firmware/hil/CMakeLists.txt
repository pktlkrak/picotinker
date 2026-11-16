cmake_minimum_required(VERSION 4.3)

add_library(pico_tinker_hil INTERFACE)

target_include_directories(pico_tinker_hil INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
