#pragma once
#ifndef CORE_GRAPHICS_VULKAN_HELPER_H
#define CORE_GRAPHICS_VULKAN_HELPER_H
#include <cinttypes>
#include <vector>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>
#include <editor/base_frame.h>

namespace core::graphics {

    void check_vk_result(VkResult err);
    bool is_extension_available(const std::vector<VkExtensionProperties>& properties, const char* extension);
#ifdef APP_USE_VULKAN_DEBUG_REPORT
    VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData);
#endif

    std::vector<const char*> vulkan_get_extensions();
    void vulkan_setup_vulkan(editor::window_base_data& data, std::vector<const char*> instance_extensions);
    void vulkan_setup_window(editor::window_base_data& data, ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height);
    int32_t vulkan_surface_bind(editor::window_base_data& data, VkSurfaceKHR& surface);
    void vulkan_framebuffer_init(editor::window_base_data& data, VkSurfaceKHR& surface, int32_t& w, int32_t& h);
    void vulkan_imgui_init(editor::window_base_data& data, ImGui_ImplVulkanH_Window* wd, float& main_scale, VkSurfaceKHR& surface);
    void vulkan_cleanup_vulkan(editor::window_base_data& data);
    void vulkan_cleanup_window(editor::window_base_data& data, ImGui_ImplVulkanH_Window* wd);
    void vulkan_frame_render(editor::window_base_data& data, ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data);
    void vulkan_frame_present(editor::window_base_data& data, ImGui_ImplVulkanH_Window* wd);
}

#endif