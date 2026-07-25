#pragma once
#ifndef EDITOR_BASE_FRAME
#define EDITOR_BASE_FRAME
#include <cinttypes>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

namespace editor {

    struct window_base_data {
        SDL_Window*              window;
        VkAllocationCallbacks*   g_Allocator = nullptr;
        VkInstance               g_Instance = VK_NULL_HANDLE;
        VkPhysicalDevice         g_PhysicalDevice = VK_NULL_HANDLE;
        VkDevice                 g_Device = VK_NULL_HANDLE;
        uint32_t                 g_QueueFamily = (uint32_t)-1;
        VkQueue                  g_Queue = VK_NULL_HANDLE;
        VkPipelineCache          g_PipelineCache = VK_NULL_HANDLE;
        VkDescriptorPool         g_DescriptorPool = VK_NULL_HANDLE;

        ImGui_ImplVulkanH_Window g_MainWindowData;
        uint32_t                 g_MinImageCount = 2;
        bool                     g_SwapChainRebuild = false;
    };

}

#endif