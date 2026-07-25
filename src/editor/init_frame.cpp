#include "init_frame.h"
#include <spdlog/spdlog.h>
#include <core/graphics/vulkan_helper.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>

namespace editor {

    int32_t window_init_start(window_init_data& data) {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
        {
            spdlog::error("Error: SDL_Init(): {}", SDL_GetError());
            return 1;
        }
        
        float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
        data.window = SDL_CreateWindow("Welcome Flamecut", (int)(1280 * main_scale), (int)(800 * main_scale), window_init_flag);

        if (data.window == nullptr)
        {
            spdlog::error("Error: SDL_CreateWindow(): {}", SDL_GetError());
            return 1;
        }

        auto extendsions = core::graphics::vulkan_get_extensions();
        VkSurfaceKHR surface;
        VkResult err;
        int32_t ferr;
        int w, h;

        core::graphics::vulkan_setup_vulkan(data, extendsions);
        ferr = core::graphics::vulkan_surface_bind(data, surface);
        if(ferr != 0) return ferr;
        core::graphics::vulkan_framebuffer_init(data, surface, w, h);
        core::graphics::vulkan_imgui_init(data, &data.g_MainWindowData, main_scale, surface);

        return 0;
    }

    int32_t window_init_run(window_init_data& data) {
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        
        bool show_demo_window = true;
        bool show_another_window = false;
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        bool done = false;
        while (!done)
        {
            // Poll and handle events (inputs, window resize, etc.)
            // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
            // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
            // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
            // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
            // [If using SDL_MAIN_USE_CALLBACKS: call ImGui_ImplSDL3_ProcessEvent() from your SDL_AppEvent() function]
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                ImGui_ImplSDL3_ProcessEvent(&event);
                if (event.type == SDL_EVENT_QUIT)
                    done = true;
                if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(data.window))
                    done = true;
            }

            // [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppIterate() function]
            if (SDL_GetWindowFlags(data.window) & SDL_WINDOW_MINIMIZED)
            {
                SDL_Delay(10);
                continue;
            }

            // Resize swap chain?
            int fb_width, fb_height;
            SDL_GetWindowSize(data.window, &fb_width, &fb_height);
            if (fb_width > 0 && fb_height > 0 && (data.g_SwapChainRebuild || data.g_MainWindowData.Width != fb_width || data.g_MainWindowData.Height != fb_height))
            {
                ImGui_ImplVulkan_SetMinImageCount(data.g_MinImageCount);
                ImGui_ImplVulkanH_CreateOrResizeWindow(data.g_Instance, data.g_PhysicalDevice, data.g_Device, &data.g_MainWindowData, data.g_QueueFamily, data.g_Allocator, fb_width, fb_height, data.g_MinImageCount, 0);
                data.g_MainWindowData.FrameIndex = 0;
                data.g_SwapChainRebuild = false;
            }

            // Start the Dear ImGui frame
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();

            // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
            if (show_demo_window)
                ImGui::ShowDemoWindow(&show_demo_window);

            // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
            {
                static float f = 0.0f;
                static int counter = 0;

                ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

                ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
                ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
                ImGui::Checkbox("Another Window", &show_another_window);

                ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
                ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

                if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                    counter++;
                ImGui::SameLine();
                ImGui::Text("counter = %d", counter);

                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
                ImGui::End();
            }

            // 3. Show another simple window.
            if (show_another_window)
            {
                ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
                ImGui::Text("Hello from another window!");
                if (ImGui::Button("Close Me"))
                    show_another_window = false;
                ImGui::End();
            }

            // Rendering
            ImGui::Render();
            ImDrawData* draw_data = ImGui::GetDrawData();
            const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
            if (!is_minimized)
            {
                data.g_MainWindowData.ClearValue.color.float32[0] = clear_color.x * clear_color.w;
                data.g_MainWindowData.ClearValue.color.float32[1] = clear_color.y * clear_color.w;
                data.g_MainWindowData.ClearValue.color.float32[2] = clear_color.z * clear_color.w;
                data.g_MainWindowData.ClearValue.color.float32[3] = clear_color.w;
                core::graphics::vulkan_frame_render(data, &data.g_MainWindowData, draw_data);
                core::graphics::vulkan_frame_present(data, &data.g_MainWindowData);
            }
        }
        return 0;
    }

    int32_t window_init_end(window_init_data& data) {
        VkResult err = vkDeviceWaitIdle(data.g_Device);
        core::graphics::check_vk_result(err);
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();

        core::graphics::vulkan_cleanup_window(data, &data.g_MainWindowData);
        core::graphics::vulkan_cleanup_vulkan(data);
        SDL_DestroyWindow(data.window);
        SDL_Quit();
        return 0;
    }

}