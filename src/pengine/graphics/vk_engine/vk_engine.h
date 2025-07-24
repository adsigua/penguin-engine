#pragma once
#ifndef PENGUIN_VK_ENGINE
#define PENGUIN_VK_ENGINE

#include <vk_types.h>
#include <vk_initializers.h>
#include <vk_vertex_data.h>
#include <VkBootstrap.h>

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

        bool isInitialized = false;

        VKEngine();

        ~VKEngine();
        void init_vk_engine(Window* window);

        void RecreateSwapChain();

        void draw(Camera camera, std::vector<RenderObject>* renderObjects);

        void WaitRendererIdle();

        void SetFrameBufferResized(bool wasResized);

        void cleanup();

        float GetSwapChainAspectRatio();

    private:
        Window* _window;

        VkInstance _instance;// Vulkan library handle
        VkDebugUtilsMessengerEXT _debug_messenger;// Vulkan debug output handle
        VkPhysicalDevice _device_physical;// GPU chosen as the default device
        VkDevice _device_logical; // Vulkan device for commands
        VkSurfaceKHR _surface;// Vulkan window surface

        VkSwapchainKHR _swapchain;
        VkFormat _swapchainImageFormat;
        VkExtent2D _swapchainExtent;
        //SwapChainData _swapChainData[SWAPCHAIN_MAX_SIZE];
        //SwapChainData _swapChainData[SWAPCHAIN_MAX_SIZE];
        std::vector<SwapChainData> _swapChainData;
        VmaAllocator _allocator;
            
        VkPhysicalDeviceProperties _device_properties_physical;

        FrameData _frames[MAX_FRAMES_IN_FLIGHT];

        QueueFamilyIndices _queue_families;

        VkQueue _graphics_queue;
        VkQueue _present_queue;
        VkQueue _transfer_queue;

        VkCommandPool _commandPool;
        VkCommandPool _transferCommandPool;

        std::vector<const char*> _requiredExtensions;
        std::vector<VkExtensionProperties> _supportedExtensions;

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        VkRenderPass _renderPass;
        VkPipelineLayout _pipelineLayout;
        VkPipeline _graphicsPipeline;


        BufferObject _vertexBufferObject;
        BufferObject _indexBufferObject;

        VkDescriptorPool _descriptorPool;
        VkDescriptorSetLayout _descriptorSetLayout;

        VkDescriptorSet _cameraDescriptorSets[MAX_FRAMES_IN_FLIGHT];
        VkDescriptorSet _descriptorSets[MAX_FRAMES_IN_FLIGHT];
        VkDescriptorSet _objectDescriptorSets[MAX_FRAMES_IN_FLIGHT];

        BufferObject _cameraUniformBufferMemory[MAX_FRAMES_IN_FLIGHT];
        BufferObject _renderOjbectsDynamicUniformBufferMemory[MAX_FRAMES_IN_FLIGHT];

        VkSampler _textureSampler;

        AllocatedImage _modelTextureImage;
        AllocatedImage _depthTextureImage;

#pragma region initialize
        void init_swapchain();

        void init_vulkan();

        void init_vma();

        void init_queue_families(vkb::Device vkbDevice);

        void init_commands();

        void init_sync_structs();
#pragma endregion

#pragma region SwapChain
        void create_swapchain(uint32_t width, uint32_t height);

        void destroy_swapchain();
#pragma endregion

#pragma region Buffers
        VkDeviceSize get_alignment(VkDeviceSize bufferSize, VkDeviceSize minBufferAlignment);

        //void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer, VmaAllocation& allocation, VmaAllocationInfo& allocationInfo);
        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, BufferObject& bufferObject);
        
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        void createAndFillBuffer(VkDeviceSize size, VkBufferUsageFlags usage, const void* data, BufferObject& bufferObject);

        VkCommandBuffer beginSingleTimeCommands();

        void endSingleTimeCommands(VkCommandBuffer commandBuffer);
#pragma endregion

#pragma region Rendering
        void createRenderPass();

        void createGraphicsPipeline();

        VkShaderModule createShaderModule(const std::vector<char>& code);

        void createFramebuffers();

        void createCommandPool();

        void createCommandBuffer();

        void record_command_buffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, std::vector<RenderObject>* renderObjects);

#pragma endregion

#pragma region Mesh buffers
        void createVertexBuffer();

        void createIndexBuffer();
#pragma endregion

#pragma region Descriptors
        void updateUniformBuffers(Camera camera, std::vector<RenderObject>* renderObjects);

        void createUniformBuffers();

        void createDescriptorPool();

        void createDescriptorSetLayout();

        void createDescriptorSets();
#pragma endregion

#pragma region Textures
        void createTextureImage();

        void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, AllocatedImage& allocatedImage);
        //void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
        
        void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
        
        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

        void createImageView(AllocatedImage& allocatedImage, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

        void createTextureImageView();

        void createTextureSampler();

        void createDepthResources();

        VkFormat findDepthFormat();

        bool hasStencilComponent(VkFormat format);

        VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
#pragma endregion

#pragma region Model
        void loadModel();
#pragma endregion

#pragma region Syncing
        void createSyncObjects();
#pragma endregion

#pragma region Getters
        FrameData& get_current_framedata();
#pragma endregion


    };
}
}
}

#endif
