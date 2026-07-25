#pragma once
#ifndef EDITOR_MAIN_FRAME
#define EDITOR_MAIN_FRAME
#include <cinttypes>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

namespace editor {

    constexpr SDL_WindowFlags window_main_flag = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;

    struct window_main_data {
        SDL_Window* window;
    };

    int32_t window_main_start();
    int32_t window_main_run();
    int32_t window_main_end();

}

#endif