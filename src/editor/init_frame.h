#pragma once
#ifndef EDITOR_INIT_FRAME
#define EDITOR_INIT_FRAME
#include <cinttypes>
#include "base_frame.h"

namespace editor {

    constexpr SDL_WindowFlags window_init_flag = SDL_WINDOW_VULKAN | SDL_WINDOW_BORDERLESS | SDL_WINDOW_HIGH_PIXEL_DENSITY;

    struct window_init_data : public window_base_data {
        
    };

    int32_t window_init_start(window_init_data& data);
    int32_t window_init_run(window_init_data& data);
    int32_t window_init_end(window_init_data& data);

}

#endif