cmake_minimum_required(VERSION 4.3)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (CMAKE_COMMAND MATCHES ".*pico-sdk.*")
        set(PICO_TINKER_EMULATOR_BUILD_DEFAULT OFF)
else()
        set(PICO_TINKER_EMULATOR_BUILD_DEFAULT ON)
endif()

option(PICO_TINKER_EMULATOR_BUILD
        "Build for emulator instead of hardware build"
        ${PICO_TINKER_EMULATOR_BUILD_DEFAULT})

if (NOT PICO_TINKER_EMULATOR_BUILD)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.1)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

        set(PICO_BOARD pico2 CACHE STRING "Board type")

        include(pico_sdk_import.cmake)

        # Must be in this file for the project to be recognized by the VS Code Extension
        pico_sdk_init()

        # Todo: Clean this up!
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -T ../../app_link.ld")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mno-pic-data-is-text-relative -mno-single-pic-base")

endif()
project(test_application C)

# Welcome in C hell :)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -shared")
add_executable(test_application
        main.c
        )

