#include "vulkan_helper.h"
#include <cinttypes>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <spdlog/spdlog.h>

namespace core::graphics {

    void check_vk_result(VkResult err)
    {
        if (err == VK_SUCCESS) {
            return;
        }
        spdlog::error("[vulkan] Error: VkResult = {}", std::to_string(err));
        if (err < 0) {
            abort();
        }
    }

    bool is_extension_available(const std::vector<VkExtensionProperties>& properties, const char* extension)
    {
        for (const VkExtensionProperties& p : properties)
            if (strcmp(p.extensionName, extension) == 0)
                return true;
        return false;
    }

#ifdef APP_USE_VULKAN_DEBUG_REPORT
    VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
    {
        (void)flags; (void)object; (void)location; (void)messageCode; (void)pUserData; (void)pLayerPrefix; // Unused arguments
        fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
        return VK_FALSE;
    }
#endif // APP_USE_VULKAN_DEBUG_REPORT

    std::vector<const char*> vulkan_get_extensions() {
        std::vector<const char*> extensions = std::vector<const char*>();
        {
            uint32_t sdl_extensions_count = 0;
            const char* const* sdl_extensions = SDL_Vulkan_GetInstanceExtensions(&sdl_extensions_count);
            for (uint32_t n = 0; n < sdl_extensions_count; n++)
                extensions.push_back(sdl_extensions[n]);
        }
        return extensions;
    }

    void vulkan_setup_vulkan(editor::window_base_data& data, std::vector<const char*> instance_extensions)
    {
        VkResult err;
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
        volkInitialize();
#endif

        // Create Vulkan Instance
        {
            VkInstanceCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

            // Enumerate available extensions
            uint32_t properties_count;
            std::vector<VkExtensionProperties> properties;
            vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, nullptr);
            properties.resize(properties_count);
            err = vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, properties.data());
            check_vk_result(err);

            // Enable required extensions
            if (is_extension_available(properties, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
                instance_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
            if (is_extension_available(properties, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME))
            {
                instance_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
                create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
            }
#endif

            // Enabling validation layers
#ifdef APP_USE_VULKAN_DEBUG_REPORT
            const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
            create_info.enabledLayerCount = 1;
            create_info.ppEnabledLayerNames = layers;
            instance_extensions.push_back("VK_EXT_debug_report");
#endif

            // Create Vulkan Instance
            create_info.enabledExtensionCount = (uint32_t)instance_extensions.size();
            create_info.ppEnabledExtensionNames = instance_extensions.data();
            err = vkCreateInstance(&create_info, data.g_Allocator, &data.g_Instance);
            check_vk_result(err);
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
            volkLoadInstance(g_Instance);
#endif

            // Setup the debug report callback
#ifdef APP_USE_VULKAN_DEBUG_REPORT
            auto f_vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(g_Instance, "vkCreateDebugReportCallbackEXT");
            IM_ASSERT(f_vkCreateDebugReportCallbackEXT != nullptr);
            VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
            debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
            debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
            debug_report_ci.pfnCallback = debug_report;
            debug_report_ci.pUserData = nullptr;
            err = f_vkCreateDebugReportCallbackEXT(g_Instance, &debug_report_ci, g_Allocator, &g_DebugReport);
            check_vk_result(err);
#endif
        }

        // Select Physical Device (GPU)
        data.g_PhysicalDevice = ImGui_ImplVulkanH_SelectPhysicalDevice(data.g_Instance);
        IM_ASSERT(data.g_PhysicalDevice != VK_NULL_HANDLE);

        // Select graphics queue family
        data.g_QueueFamily = ImGui_ImplVulkanH_SelectQueueFamilyIndex(data.g_PhysicalDevice);
        IM_ASSERT(data.g_QueueFamily != (uint32_t)-1);

        // Create Logical Device (with 1 queue)
        {
            ImVector<const char*> device_extensions;
            device_extensions.push_back("VK_KHR_swapchain");

            // Enumerate physical device extension
            uint32_t properties_count;
            ImVector<VkExtensionProperties> properties;
            vkEnumerateDeviceExtensionProperties(data.g_PhysicalDevice, nullptr, &properties_count, nullptr);
            properties.resize(properties_count);
            vkEnumerateDeviceExtensionProperties(data.g_PhysicalDevice, nullptr, &properties_count, properties.Data);
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
            if (IsExtensionAvailable(properties, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
                device_extensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

            const float queue_priority[] = { 1.0f };
            VkDeviceQueueCreateInfo queue_info[1] = {};
            queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_info[0].queueFamilyIndex = data.g_QueueFamily;
            queue_info[0].queueCount = 1;
            queue_info[0].pQueuePriorities = queue_priority;
            VkDeviceCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            create_info.queueCreateInfoCount = sizeof(queue_info) / sizeof(queue_info[0]);
            create_info.pQueueCreateInfos = queue_info;
            create_info.enabledExtensionCount = (uint32_t)device_extensions.Size;
            create_info.ppEnabledExtensionNames = device_extensions.Data;
            err = vkCreateDevice(data.g_PhysicalDevice, &create_info, data.g_Allocator, &data.g_Device);
            check_vk_result(err);
            vkGetDeviceQueue(data.g_Device, data.g_QueueFamily, 0, &data.g_Queue);
        }

        // Create Descriptor Pool
        // If you wish to load e.g. additional textures you may need to alter pools sizes and maxSets.
        {
            VkDescriptorPoolSize pool_sizes[] =
            {
                { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, IMGUI_IMPL_VULKAN_MINIMUM_SAMPLED_IMAGE_POOL_SIZE },
                { VK_DESCRIPTOR_TYPE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_SAMPLER_POOL_SIZE },
            };
            VkDescriptorPoolCreateInfo pool_info = {};
            pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            pool_info.maxSets = 0;
            for (VkDescriptorPoolSize& pool_size : pool_sizes)
                pool_info.maxSets += pool_size.descriptorCount;
            pool_info.poolSizeCount = (uint32_t)IM_COUNTOF(pool_sizes);
            pool_info.pPoolSizes = pool_sizes;
            err = vkCreateDescriptorPool(data.g_Device, &pool_info, data.g_Allocator, &data.g_DescriptorPool);
            check_vk_result(err);
        }
    }

    void vulkan_setup_window(editor::window_base_data& data, ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height)
    {
        // Check for WSI support
        VkBool32 res;
        vkGetPhysicalDeviceSurfaceSupportKHR(data.g_PhysicalDevice, data.g_QueueFamily, surface, &res);
        if (res != VK_TRUE)
        {
            fprintf(stderr, "Error no WSI support on physical device 0\n");
            exit(-1);
        }

        // Select Surface Format
        const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
        const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        wd->Surface = surface;
        wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(data.g_PhysicalDevice, wd->Surface, requestSurfaceImageFormat, (size_t)IM_COUNTOF(requestSurfaceImageFormat), requestSurfaceColorSpace);

        // Select Present Mode
#ifdef APP_USE_UNLIMITED_FRAME_RATE
        VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
#else
        VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
#endif
        wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(data.g_PhysicalDevice, wd->Surface, &present_modes[0], IM_COUNTOF(present_modes));
        //printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

        // Create SwapChain, RenderPass, Framebuffer, etc.
        IM_ASSERT(data.g_MinImageCount >= 2);
        ImGui_ImplVulkanH_CreateOrResizeWindow(data.g_Instance, data.g_PhysicalDevice, data.g_Device, wd, data.g_QueueFamily, data.g_Allocator, width, height, data.g_MinImageCount, 0);
    }

    int32_t vulkan_surface_bind(editor::window_base_data& data, VkSurfaceKHR& surface) {
        if (SDL_Vulkan_CreateSurface(data.window, data.g_Instance, data.g_Allocator, &surface) == 0)
        {
            spdlog::error("Failed to create Vulkan surface.");
            return 1;
        }
        return 0;
    }

    void vulkan_framebuffer_init(editor::window_base_data& data, VkSurfaceKHR& surface, int32_t& w, int32_t& h) {
        SDL_GetWindowSize(data.window, &w, &h);
        ImGui_ImplVulkanH_Window* wd = &data.g_MainWindowData;
        vulkan_setup_window(data, wd, surface, w, h);
        SDL_SetWindowPosition(data.window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        SDL_ShowWindow(data.window);
    }

    void vulkan_imgui_init(editor::window_base_data& data, ImGui_ImplVulkanH_Window* wd, float& main_scale, VkSurfaceKHR& surface) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsLight();

        // Setup scaling
        ImGuiStyle& style = ImGui::GetStyle();
        style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
        style.FontScaleDpi = main_scale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)

        // Setup Platform/Renderer backends
        ImGui_ImplSDL3_InitForVulkan(data.window);
        ImGui_ImplVulkan_InitInfo init_info = {};
        //init_info.ApiVersion = VK_API_VERSION_1_3;              // Pass in your value of VkApplicationInfo::apiVersion, otherwise will default to header version.
        init_info.Instance = data.g_Instance;
        init_info.PhysicalDevice = data.g_PhysicalDevice;
        init_info.Device = data.g_Device;
        init_info.QueueFamily = data.g_QueueFamily;
        init_info.Queue = data.g_Queue;
        init_info.PipelineCache = data.g_PipelineCache;
        init_info.DescriptorPool = data.g_DescriptorPool;
        init_info.MinImageCount = data.g_MinImageCount;
        init_info.ImageCount = wd->ImageCount;
        init_info.Allocator = data.g_Allocator;
        init_info.PipelineInfoMain.RenderPass = wd->RenderPass;
        init_info.PipelineInfoMain.Subpass = 0;
        init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.CheckVkResultFn = core::graphics::check_vk_result;
        ImGui_ImplVulkan_Init(&init_info);
    }

    void vulkan_cleanup_vulkan(editor::window_base_data& data)
    {
        vkDestroyDescriptorPool(data.g_Device, data.g_DescriptorPool, data.g_Allocator);

    #ifdef APP_USE_VULKAN_DEBUG_REPORT
        // Remove the debug report callback
        auto f_vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(g_Instance, "vkDestroyDebugReportCallbackEXT");
        f_vkDestroyDebugReportCallbackEXT(g_Instance, g_DebugReport, g_Allocator);
    #endif // APP_USE_VULKAN_DEBUG_REPORT

        vkDestroyDevice(data.g_Device, data.g_Allocator);
        vkDestroyInstance(data.g_Instance, data.g_Allocator);
    }

    void vulkan_cleanup_window(editor::window_base_data& data, ImGui_ImplVulkanH_Window* wd)
    {
        ImGui_ImplVulkanH_DestroyWindow(data.g_Instance, data.g_Device, wd, data.g_Allocator);
        vkDestroySurfaceKHR(data.g_Instance, wd->Surface, data.g_Allocator);
    }

    void vulkan_frame_render(editor::window_base_data& data, ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data)
    {
        VkSemaphore image_acquired_semaphore  = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
        VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
        VkResult err = vkAcquireNextImageKHR(data.g_Device, wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wd->FrameIndex);
        if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
            data.g_SwapChainRebuild = true;
        if (err == VK_ERROR_OUT_OF_DATE_KHR)
            return;
        if (err != VK_SUBOPTIMAL_KHR)
            check_vk_result(err);

        ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
        {
            err = vkWaitForFences(data.g_Device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking
            check_vk_result(err);

            err = vkResetFences(data.g_Device, 1, &fd->Fence);
            check_vk_result(err);
        }
        {
            err = vkResetCommandPool(data.g_Device, fd->CommandPool, 0);
            check_vk_result(err);
            VkCommandBufferBeginInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
            check_vk_result(err);
        }
        {
            VkRenderPassBeginInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            info.renderPass = wd->RenderPass;
            info.framebuffer = fd->Framebuffer;
            info.renderArea.extent.width = wd->Width;
            info.renderArea.extent.height = wd->Height;
            info.clearValueCount = 1;
            info.pClearValues = &wd->ClearValue;
            vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
        }

        // Record dear imgui primitives into command buffer
        ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

        // Submit command buffer
        vkCmdEndRenderPass(fd->CommandBuffer);
        {
            VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            VkSubmitInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            info.waitSemaphoreCount = 1;
            info.pWaitSemaphores = &image_acquired_semaphore;
            info.pWaitDstStageMask = &wait_stage;
            info.commandBufferCount = 1;
            info.pCommandBuffers = &fd->CommandBuffer;
            info.signalSemaphoreCount = 1;
            info.pSignalSemaphores = &render_complete_semaphore;

            err = vkEndCommandBuffer(fd->CommandBuffer);
            check_vk_result(err);
            err = vkQueueSubmit(data.g_Queue, 1, &info, fd->Fence);
            check_vk_result(err);
        }
    }

    void vulkan_frame_present(editor::window_base_data& data, ImGui_ImplVulkanH_Window* wd)
    {
        if (data.g_SwapChainRebuild)
            return;
        VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
        VkPresentInfoKHR info = {};
        info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &render_complete_semaphore;
        info.swapchainCount = 1;
        info.pSwapchains = &wd->Swapchain;
        info.pImageIndices = &wd->FrameIndex;
        VkResult err = vkQueuePresentKHR(data.g_Queue, &info);
        if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
            data.g_SwapChainRebuild = true;
        if (err == VK_ERROR_OUT_OF_DATE_KHR)
            return;
        if (err != VK_SUBOPTIMAL_KHR)
            check_vk_result(err);
        wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->SemaphoreCount; // Now we can use the next set of semaphores
    }

}