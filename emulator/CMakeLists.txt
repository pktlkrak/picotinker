cmake_minimum_required(VERSION 4.3)

# On Linux use system SDL by default
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(PICO_TINKER_USE_SYSTEM_SDL_DEFAULT ON)
else()
    set(PICO_TINKER_USE_SYSTEM_SDL_DEFAULT OFF)
endif()

option(PICO_TINKER_USE_SYSTEM_SDL
        "Find SDL in system instead of building it from source"
        ${PICO_TINKER_USE_SYSTEM_SDL_DEFAULT})

if(PICO_TINKER_USE_SYSTEM_SDL)
        find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
else()
        set(SDL_SHARED OFF)
        set(SDL_STATIC ON)
        set(SDL3IMAGE_AVIF OFF)

        include(FetchContent)
        FetchContent_Declare(
                SDL
                GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
                GIT_TAG origin/main
        )
        FetchContent_MakeAvailable(SDL)
endif()

add_library(pico_tinker_emulator
        hil.c
        window.c
        )

target_link_libraries(pico_tinker_emulator PRIVATE
        pico_tinker_hil
        SDL3::SDL3
        )
