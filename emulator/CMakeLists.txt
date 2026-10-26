cmake_minimum_required(VERSION 4.3)

add_library(pico_tinker_emulator
        hil.c
        )

target_link_libraries(pico_tinker_emulator
        pico_tinker_hil
        )
