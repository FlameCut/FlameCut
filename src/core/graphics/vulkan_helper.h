#pragma once
#ifndef CORE_GRAPHICS_VULKAN_HELPER_H
#define CORE_GRAPHICS_VULKAN_HELPER_H
#include <vector>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>
#include <editor/base_frame.h>

namespace core::graphics {

    void check_vk_result(VkResult err);
    bool is_extension_available(const std::vector<VkExtensionProperties>& properties, const char* extension);
    std::vector<const char*> vulkan_get_extensions();
    void vulkan_setup_vulkan(editor::window_base_data& data, std::vector<const char*> instance_extensions);
    void vulkan_setup_window(editor::window_base_data& data, ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height);
}

#endif