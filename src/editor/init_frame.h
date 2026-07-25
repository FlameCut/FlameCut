#pragma once
#ifndef EDITOR_INIT_FRAME
#define EDITOR_INIT_FRAME
#include <cinttypes>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

namespace editor {

    constexpr SDL_WindowFlags window_init_flag = SDL_WINDOW_VULKAN | SDL_WINDOW_BORDERLESS | SDL_WINDOW_HIGH_PIXEL_DENSITY;

    struct window_init_data {
        SDL_Window* window;
    };

    int32_t window_init_start();
    int32_t window_init_run();
    int32_t window_init_end();

}

#endif