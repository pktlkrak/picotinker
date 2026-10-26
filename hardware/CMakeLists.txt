cmake_minimum_required(VERSION 4.3)

add_library(pico_tinker_hardware
        hil.c
        )

target_link_libraries(pico_tinker_hardware PRIVATE
        pico_stdlib
        pico_tinker_hil
        )
