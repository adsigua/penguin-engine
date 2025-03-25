#pragma once
#ifndef PENGUIN_VK_ENGINE
#define PENGUIN_VK_ENGINE

#include "vk_types.h"

namespace PenguinEngine {
namespace Graphics {
namespace Vulkan {

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

        VKEngine();

        ~VKEngine();

        void InitVulkan(GLFWwindow* window);

        void RecreateSwapChain();

        void DrawFrame(Camera camera, std::vector<RenderObject>* renderObjects);

        void WaitRendererIdle();

        void SetFrameBufferResized(bool wasResized);

        void Cleanup();

        float GetSwapChainAspectRatio();

    private:
        GLFWwindow* _window;

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        VkDebugUtilsMessengerEXT _debugMessenger;
        VkInstance _instance;

        VmaAllocator _allocator;
            
        VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
        VkDevice _device;
        VkPhysicalDeviceProperties _physicalDeviceProperties;

        VkSurfaceKHR _surface;
        VkQueue _graphicsQueue;
        VkQueue _presentQueue;
        VkQueue _transferQueue;

        VkCommandPool _commandPool;
        VkCommandPool _transferCommandPool;

        std::vector<const char*> _requiredExtensions;
        std::vector<VkExtensionProperties> _supportedExtensions;

        VkSwapchainKHR _swapChain;
        VkFormat _swapChainImageFormat;

        VkExtent2D _swapChainExtent;

        VkRenderPass _renderPass;
        VkPipelineLayout _pipelineLayout;
        VkPipeline _graphicsPipeline;

        FrameData _frames[MAX_FRAMES_IN_FLIGHT];

        SwapChainData _swapChainData[SWAPCHAIN_MAX_SIZE];
        //std::vector<SwapChainData> _swapChainData;

        //VkBuffer _vertexBuffer;
        //VkDeviceMemory _vertexBufferMemory;
        //VkBuffer _indexBuffer;
        //VkDeviceMemory _indexBufferMemory;
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

        //VkImage _depthImage;
        //VkDeviceMemory _depthImageMemory;
        //VkImageView _depthImageView;

        //std::vector<SwapChainData> _swapChainData;
        //std::vector<VkFramebuffer> _swapChainFrameBuffers;
        //bool _framebufferResized = false;
        //uint32_t _currentFrame = 0;

#pragma region initialize
        void createInstance();

        bool checkValidationLayerSupport();

        void InitExtensions();

        bool isExtensionSupported(const char* extensionName);

        void AddIfSupportedExtension(const char* extensionName, uint32_t extensionFlag, VkInstanceCreateInfo* createInfo);

        void PrintInstanceRequiredExtensions(VkInstanceCreateInfo createInfo);

        void PrintSupportedExtensions();

        void pickPhysicalDevice();

        bool isDeviceSuitable(VkPhysicalDevice device);

        bool checkDeviceExtensionSupport(VkPhysicalDevice device);

        int rateDeviceSuitability(VkPhysicalDevice device);

        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

        void createLogicalDevice(); 

        void createSurface();

        void createSurfaceVulkan(GLFWwindow* window);

        void initVMA();
#pragma endregion

#pragma region Validation Layers

        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

        void setupDebugMessenger();
#pragma endregion

#pragma region SwapChain
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

        void createSwapChain();

        void createSwapChainImageViews();

        void cleanupSwapChain();
#pragma endregion

#pragma region Buffers
        VkDeviceSize getAlignment(VkDeviceSize bufferSize, VkDeviceSize minBufferAlignment);

        //void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer, VmaAllocation& allocation, VmaAllocationInfo& allocationInfo);
        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, BufferObject& bufferObject);
        
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

        void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, std::vector<RenderObject>* renderObjects);

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
        FrameData& GetCurrentFrameData();
#pragma endregion


    };
}
}
}

#endif
