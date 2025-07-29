#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <VkBootstrap.h>
#include <pengine/core/camera.h>
#include <pengine/core/window.h>
#include <pengine/core/render_object.h>

#include "vk_types.h"
#include "vk_descriptors.h"

#include <functional>

namespace penguin_engine {
namespace graphics {
namespace vulkan {

    #ifdef NDEBUG
    const bool enableValidationLayers = false;
    #else
    const bool enableValidationLayers = true;
    #endif

    const int MAX_FRAMES_IN_FLIGHT = 2;
    const int MAX_INSTANCE_COUNT = 100;

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };


    class VKEngine {
    public:
        const static int SWAPCHAIN_MAX_SIZE = 5;

        void init_vk_engine(Window* window);

        void draw(Camera camera, std::vector<RenderObject>* renderObjects);

        void cleanup();

        void recreate_swapchain();

        void set_framebuffer_was_resized(bool wasResized);

        ComputeEffect* get_current_background_fx();

        int* get_background_fx_current_index();

        int get_background_fx_count();

        float get_swapchain_aspect_ratio();

        GPUMeshBuffers upload_mesh_buffer(std::span<uint32_t> indices, std::span<Vertex> vertices);
    private:
        bool _isInitialized = false;
        uint32_t _current_frame_number = 0;
        uint32_t _current_frame_index = 0;
        Window* _window;

        VkInstance _instance;// Vulkan library handle
        VkDebugUtilsMessengerEXT _debug_messenger;// Vulkan debug output handle
        VkPhysicalDevice _physical_device;// GPU chosen as the default device
        VkDevice _logical_device; // Vulkan device for commands
        VkSurfaceKHR _surface;// Vulkan window surface

        VkSwapchainKHR _swapchain;
        VkFormat _swapchain_image_format;
        VkExtent2D _swapchain_extent;
        std::vector<SwapChainData> _swapchain_data;
        VmaAllocator _vma_allocator;
            
        VkPhysicalDeviceProperties _device_properties_physical;

        FrameData _frames[MAX_FRAMES_IN_FLIGHT];

        QueueFamilyIndices _queue_families;

        DrawImageData _draw_image_data;
        AllocatedImage _depth_image;

        VkExtent2D _draw_extent;

        VkQueue _graphics_queue;
        VkQueue _present_queue;
        VkQueue _transfer_queue;

        DescriptorAllocator _descriptor_allocator;

        ImgGUISyncData _imgui_sync_data;

        VkPipelineLayout _background_pipeline_layout;
        //VkPipeline _gradient_pipeline;
        VkPipelineLayout _triangle_pipeline_layout;
        VkPipeline _triangle_pipeline;

        VkPipelineLayout _mesh_pipeline_layout;
        VkPipeline _mesh_pipeline;

        std::vector<ComputeEffect> _backgroundEffects;
        int _currentBackgroundEffect{ 0 };

        GPUMeshBuffers rectangle;
        std::vector<std::shared_ptr<GPUMeshAsset>> testMeshes;

#pragma region initialize
        void init_vulkan();

        void init_swapchain();

        void init_vma();

        void init_queue_families(vkb::Device vkbDevice);

        void init_commands();

        void init_sync_structs();

        void init_descriptors();

        void init_pipelines();

        void init_imgui();

        void init_default_data();
       
#pragma endregion

#pragma region SwapChain
        void create_swapchain(uint32_t width, uint32_t height);

        void init_draw_image();

        void destroy_swapchain();
#pragma endregion

#pragma region Command Utils
        void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);
#pragma endregion

#pragma region Pipelines
        void init_background_pipeline();

        void init_triangle_pipeline();

        void init_mesh_pipeline();
#pragma endregion

#pragma region Rendering
        void draw_imgui(VkCommandBuffer cmd, VkImageView targetImageView);

        VkShaderModule createShaderModule(const std::vector<char>& code);

        void record_command_buffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, std::vector<RenderObject>* renderObjects);

        void draw_background(VkCommandBuffer cmd);

        void draw_geometry(VkCommandBuffer cmd);
#pragma endregion

#pragma region Buffers
        AllocatedBuffer create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

        
#pragma endregion

#pragma region Getters
        FrameData& get_current_framedata() {
            return _frames[_current_frame_index];
        }
#pragma endregion
    };
}
}
}

