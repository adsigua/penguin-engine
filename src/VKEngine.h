#pragma once
#ifndef PENGUIN_VK_ENGINE
#define PENGUIN_VK_ENGINE

#define GLFW_INCLUDE_VULKAN
#define NOMINMAX

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "VMAUsage.h"
#include "vk_mem_alloc.h"

//#include "VKInit.h"
#include "VKTypes.h"
#include "VertexData.h"
#include "RenderObject.h"
#include "Camera.h"

namespace PenguinEngine {
namespace Graphics {

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

        ~VKEngine() {
            
        }

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
        //std::vector<VkImage> _swapChainImages;
        VkFormat _swapChainImageFormat;

        //std::vector<VkImageView> _swapChainImageViews;
        VkExtent2D _swapChainExtent;

        VkRenderPass _renderPass;
        VkPipelineLayout _pipelineLayout;
        VkPipeline _graphicsPipeline;

        FrameData _frames[MAX_FRAMES_IN_FLIGHT];

        //std::vector<VkFramebuffer> _swapChainFramebuffers;
        //std::vector<VkFramebuffer> _swapChainFramebuffers;
        //std::vector<VkFramebuffer> _swapChainFramebuffers;
        //VkFramebuffer _swapChainFramebuffers[5];
        //VkFramebuffer *_swapChainFramebuffers;

        std::vector<SwapChainData> _swapChainData;

        VkBuffer _vertexBuffer;
        VkDeviceMemory _vertexBufferMemory;
        VkBuffer _indexBuffer;
        VkDeviceMemory _indexBufferMemory;

        VkDescriptorPool _descriptorPool;
        VkDescriptorSetLayout _descriptorSetLayout;

        VkDescriptorSet _cameraDescriptorSets[MAX_FRAMES_IN_FLIGHT];
        VkDescriptorSet _descriptorSets[MAX_FRAMES_IN_FLIGHT];
        VkDescriptorSet _objectDescriptorSets[MAX_FRAMES_IN_FLIGHT];

        UniformBufferMemory _cameraUniformBufferMemory[MAX_FRAMES_IN_FLIGHT];
        UniformBufferMemory _renderOjbectsDynamicUniformBufferMemory[MAX_FRAMES_IN_FLIGHT];

        VkImage _textureImage;
        VkDeviceMemory _textureImageMemory;
        VkImageView _textureImageView;
        VkSampler _textureSampler;

        VkImage _depthImage;
        VkDeviceMemory _depthImageMemory;
        VkImageView _depthImageView;

        //bool _framebufferResized = false;
        //uint32_t _currentFrame = 0;

#pragma region Init
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

        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

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

        void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

        VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

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

#endif
