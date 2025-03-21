#include "VKEngine.h"

#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <stdlib.h>     /* srand, rand */
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <set>
#include <chrono>
#include <cstring>
#include <memory>

#include "../TransformObject.h"
#include "../Transform.h"

namespace PenguinEngine {
namespace Graphics {

    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    const std::string MODEL_PATH = "models/viking_room.obj";
    const std::string TEXTURE_PATH = "textures/viking_room.png";

    bool _framebufferResized = false;
    uint32_t _currentFrame = 0;
    int _swapChainSize = 0;

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }


    static std::vector<char> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();
        return buffer;
    }

    static constexpr uint32_t GetVulkanApiVersion()
    {
#if VMA_VULKAN_VERSION == 1003000
        return VK_API_VERSION_1_3;
#elif VMA_VULKAN_VERSION == 1002000
        return VK_API_VERSION_1_2;
#elif VMA_VULKAN_VERSION == 1001000
        return VK_API_VERSION_1_1;
#elif VMA_VULKAN_VERSION == 1000000
        return VK_API_VERSION_1_0;
#else
#error Invalid VMA_VULKAN_VERSION.
        return UINT32_MAX;
#endif
    }


    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {

        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

#pragma region Public
    VKEngine::VKEngine(){
    }

    VKEngine::~VKEngine() {
    }

    void VKEngine::InitVulkan(GLFWwindow* window)
    {
        createInstance();
        setupDebugMessenger();

        _window = window;

        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();

        initVMA();

        createSwapChain();
        createSwapChainImageViews();

        createRenderPass();

        createCommandPool();

        createDepthResources();
        createFramebuffers();

        createTextureImage();
        createTextureImageView();
        createTextureSampler();

        loadModel();
        createVertexBuffer();
        createIndexBuffer();

        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSetLayout();
        createDescriptorSets();

        createGraphicsPipeline();

        createCommandBuffer();

        createSyncObjects();
    }

    void VKEngine::RecreateSwapChain() {
        int width = 0, height = 0;
        glfwGetFramebufferSize(_window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(_window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(_device);

        cleanupSwapChain();
        _depthTextureImage.DestroyAllocatedImage(_device, _allocator);

        createSwapChain();
        createSwapChainImageViews();
        createDepthResources();
        createFramebuffers();
    }

    void VKEngine::DrawFrame(Camera camera, std::vector<RenderObject>* renderObjects) {
        FrameData currentFrameData = GetCurrentFrameData();
        vkWaitForFences(_device, 1, &currentFrameData.renderFence, VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(_device, _swapChain, UINT64_MAX, currentFrameData.presentSemaphore, VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            RecreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        updateUniformBuffers(camera, renderObjects);

        vkResetFences(_device, 1, &currentFrameData.renderFence);

        vkResetCommandBuffer(currentFrameData.commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
        recordCommandBuffer(currentFrameData.commandBuffer, imageIndex, renderObjects);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { currentFrameData.presentSemaphore };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &currentFrameData.commandBuffer;

        VkSemaphore signalSemaphores[] = { currentFrameData.renderSemaphore };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        VkResult queueSubmitResult = vkQueueSubmit(_graphicsQueue, 1, &submitInfo, currentFrameData.renderFence);
        if (queueSubmitResult != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { _swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(_presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || _framebufferResized) {
            _framebufferResized = false;
            RecreateSwapChain();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        _currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void VKEngine::WaitRendererIdle() {
        vkDeviceWaitIdle(_device);
    }

    void VKEngine::SetFrameBufferResized(bool wasResized) {
        _framebufferResized = wasResized;
    }

    void VKEngine::Cleanup() {
        //vkWaitForFences(_device, 1, &GetCurrentFrameData().renderFence, true, 1000000000);

        cleanupSwapChain();

        vkDestroyPipeline(_device, _graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);
        vkDestroyRenderPass(_device, _renderPass, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            _cameraUniformBufferMemory[i].DestroyBufferObject(_allocator);
            _renderOjbectsDynamicUniformBufferMemory[i].DestroyBufferObject(_allocator);
        }

        vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);

        /*vkDestroyBuffer(_device, _indexBuffer, nullptr);
        vkFreeMemory(_device, _indexBufferMemory, nullptr);
        vkDestroyBuffer(_device, _vertexBuffer, nullptr);
        vkFreeMemory(_device, _vertexBufferMemory, nullptr);*/

        vkDestroySampler(_device, _textureSampler, nullptr);
        _modelTextureImage.DestroyAllocatedImage(_device, _allocator);
        _depthTextureImage.DestroyAllocatedImage(_device, _allocator);
        vkDestroyDescriptorSetLayout(_device, _descriptorSetLayout, nullptr);

        _vertexBufferObject.DestroyBufferObject(_allocator);
        _indexBufferObject.DestroyBufferObject(_allocator);

        //vkDestroyImageView(_device, _textureImageView, nullptr);
        //vkDestroyImage(_device, _textureImage, nullptr);
        //vkFreeMemory(_device, _textureImageMemory, nullptr);

        //vkDestroyImageView(_device, _depthImageView, nullptr);
        //vkDestroyImage(_device, _depthImage, nullptr);
        //vkFreeMemory(_device, _depthImageMemory, nullptr);


        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            _frames[i].DestroyFrameData(_device);
        }

        vkDestroyCommandPool(_device, _commandPool, nullptr);
        vkDestroyCommandPool(_device, _transferCommandPool, nullptr);

        //delete[] _swapChainFramebuffers;
        //std::vector<VkFramebuffer>().swap(_swapChainFramebuffers);

        vmaDestroyAllocator(_allocator);

        vkDestroyDevice(_device, nullptr);

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(_instance, _surface, nullptr);
        vkDestroyInstance(_instance, nullptr);
    }

    float VKEngine::GetSwapChainAspectRatio() {
        return _swapChainExtent.width / (float)_swapChainExtent.height;
    }
#pragma endregion
 
#pragma region Init
    void VKEngine::createInstance() {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        InitExtensions();
        createInfo.enabledExtensionCount = (uint32_t)_requiredExtensions.size();
        createInfo.ppEnabledExtensionNames = _requiredExtensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers) {
            //createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            //createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else {
            //createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        /*AddIfSupportedExtension(
            VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
            VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
            &createInfo);*/

            //PrintInstanceRequiredExtensions(createInfo);
            //PrintSupportedExtensions();

        createInfo.enabledLayerCount = 0;

        if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

    bool VKEngine::checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    void VKEngine::InitExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        for (uint32_t i = 0; i < glfwExtensionCount; i++) {
            _requiredExtensions.emplace_back(glfwExtensions[i]);
        }

        if (enableValidationLayers) {
            _requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        //available extensions
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        _supportedExtensions = std::vector<VkExtensionProperties>(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, _supportedExtensions.data());

        int toAddExtensionCount = glfwExtensionCount - 1;
        while (toAddExtensionCount >= 0) {
            if (isExtensionSupported(_requiredExtensions[toAddExtensionCount])) {
                toAddExtensionCount--;
            }
            else {
                std::cout << "Extension " << _requiredExtensions[toAddExtensionCount] << " not supported!" << '\n';
                _requiredExtensions.erase(_requiredExtensions.begin() + toAddExtensionCount);
            }
        }
    }

    bool VKEngine::isExtensionSupported(const char* extensionName) {
        for (const auto& extension : _supportedExtensions) {
            if (std::strcmp(extensionName, extension.extensionName) == 0) {
                return true;
            }
        }
        return false;
    }

    void VKEngine::AddIfSupportedExtension(const char* extensionName, uint32_t extensionFlag, VkInstanceCreateInfo* createInfo)
    {
        if (isExtensionSupported(extensionName)) {
            _requiredExtensions.emplace_back(extensionName);
            (*createInfo).flags |= extensionFlag;

            (*createInfo).enabledExtensionCount = (uint32_t)_requiredExtensions.size();
            (*createInfo).ppEnabledExtensionNames = _requiredExtensions.data();
        }
        else {
            std::cout << "Extension " << extensionName << " not supported!" << '\n';
        }
    }

    void VKEngine::PrintInstanceRequiredExtensions(VkInstanceCreateInfo createInfo) {
        std::cout << "available extensions:\n";

        for (uint32_t i = 0; i < createInfo.enabledExtensionCount; i++) {
            std::cout << '\t' << createInfo.ppEnabledExtensionNames[i] << '\n';
        }
    }

    void VKEngine::PrintSupportedExtensions() {
        std::cout << "supported extensions:\n";

        for (const auto& extension : _supportedExtensions) {
            std::cout << '\t' << extension.extensionName << '\n';
        }
    }

    void VKEngine::pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

        //// Use an ordered map to automatically sort candidates by increasing score
        std::multimap<int, VkPhysicalDevice> candidates;
        ////std::cout << "devices found:\n";

        for (const auto& device : devices) {
            int score = rateDeviceSuitability(device);
            candidates.insert(std::make_pair(score, device));
        }

        // Check if the best candidate is suitable at all
        if (candidates.rbegin()->first > 0) {
            _physicalDevice = candidates.rbegin()->second;
        }
        else {
            throw std::runtime_error("failed to find a suitable GPU!");
        }

        /*for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                _physicalDevice = device;
                break;
            }
        }*/

        if (_physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }

        vkGetPhysicalDeviceProperties(_physicalDevice, &_physicalDeviceProperties);
    }

    bool VKEngine::isDeviceSuitable(VkPhysicalDevice device) {
        QueueFamilyIndices indices = findQueueFamilies(device);
        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }

    bool VKEngine::checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    int VKEngine::rateDeviceSuitability(VkPhysicalDevice device) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
        
        QueueFamilyIndices indices = findQueueFamilies(device);
        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        //std::cout << '\t' << deviceProperties.deviceName << '\n';

        int score = 0;

        // Discrete GPUs have a significant performance advantage
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 1000;
        }

        // Maximum possible size of textures affects graphics quality
        score += deviceProperties.limits.maxImageDimension2D;

        bool minimumReq = indices.isComplete() && extensionsSupported && swapChainAdequate && deviceFeatures.samplerAnisotropy;

        //// Application can't function without geometry shaders
        if (!minimumReq) {
            return 0;
        }

        return score;
    }

    QueueFamilyIndices VKEngine::findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT & ~VK_QUEUE_GRAPHICS_BIT) {
                indices.transferFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &presentSupport);

            if (presentSupport) {
                indices.presentFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }
            i++;
        }
        return indices;
    }

    void VKEngine::createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(_physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        //if (enableValidationLayers) {
        //    //createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        //    //createInfo.ppEnabledLayerNames = validationLayers.data();
        //}
        //else {
        //    //createInfo.enabledLayerCount = 0;
        //}

        if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(_device, indices.graphicsFamily.value(), 0, &_graphicsQueue);
        vkGetDeviceQueue(_device, indices.presentFamily.value(), 0, &_presentQueue);
        vkGetDeviceQueue(_device, indices.transferFamily.value(), 0, &_transferQueue);
    }

    void VKEngine::createSurface() {
        if (glfwCreateWindowSurface(_instance, _window, nullptr, &_surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void VKEngine::createSurfaceVulkan(GLFWwindow* window) {
        VkWin32SurfaceCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.hwnd = glfwGetWin32Window(window);
        createInfo.hinstance = GetModuleHandle(nullptr);

        if (vkCreateWin32SurfaceKHR(_instance, &createInfo, nullptr, &_surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void VKEngine::initVMA() {
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.vulkanApiVersion = GetVulkanApiVersion();
        allocatorInfo.physicalDevice = _physicalDevice;
        allocatorInfo.device = _device;
        allocatorInfo.instance = _instance;
        allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

        vmaCreateAllocator(&allocatorInfo, &_allocator);
    }

#pragma endregion

#pragma region Validation Layers
            void VKEngine::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
                createInfo = {};
                createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
                //createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
                createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT  ;
                createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                createInfo.pfnUserCallback = debugCallback;
            }

            void VKEngine::setupDebugMessenger() {
                if (!enableValidationLayers) return;

                VkDebugUtilsMessengerCreateInfoEXT createInfo;
                populateDebugMessengerCreateInfo(createInfo);

                if (CreateDebugUtilsMessengerEXT(_instance, &createInfo, nullptr, &_debugMessenger) != VK_SUCCESS) {
                    throw std::runtime_error("failed to set up debug messenger!");
                }
            }
#pragma endregion

#pragma region SwapChain
            SwapChainSupportDetails VKEngine::querySwapChainSupport(VkPhysicalDevice device) {
                SwapChainSupportDetails details;

                vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &details.capabilities);

                uint32_t formatCount;
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, nullptr);

                if (formatCount != 0) {
                    details.formats.resize(formatCount);
                    vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, details.formats.data());
                }

                uint32_t presentModeCount;
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, nullptr);

                if (presentModeCount != 0) {
                    details.presentModes.resize(presentModeCount);
                    vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, details.presentModes.data());
                }

                return details;
            }

            VkSurfaceFormatKHR VKEngine::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
                for (const auto& availableFormat : availableFormats) {
                    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                        return availableFormat;
                    }
                }
                return availableFormats[0];
            }

            VkPresentModeKHR VKEngine::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
                for (const auto& availablePresentMode : availablePresentModes) {
                    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                        return availablePresentMode;
                    }
                }

                return VK_PRESENT_MODE_FIFO_KHR;
            }

            VkExtent2D VKEngine::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
                if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
                    return capabilities.currentExtent;
                }
                else {
                    int width, height;
                    glfwGetFramebufferSize(window, &width, &height);

                    VkExtent2D actualExtent = {
                        static_cast<uint32_t>(width),
                        static_cast<uint32_t>(height)
                    };

                    actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
                    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

                    return actualExtent;
                }
            }

            void VKEngine::createSwapChain() {
                SwapChainSupportDetails swapChainSupport = querySwapChainSupport(_physicalDevice);

                VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
                VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
                VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, _window);

                uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
                if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
                    imageCount = swapChainSupport.capabilities.maxImageCount;
                }
                VkSwapchainCreateInfoKHR createInfo{};
                createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
                createInfo.surface = _surface;

                createInfo.minImageCount = imageCount;
                createInfo.imageFormat = surfaceFormat.format;
                createInfo.imageColorSpace = surfaceFormat.colorSpace;
                createInfo.imageExtent = extent;
                createInfo.imageArrayLayers = 1;
                createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

                QueueFamilyIndices indices = findQueueFamilies(_physicalDevice);
                uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

                if (indices.graphicsFamily != indices.presentFamily) {
                    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                    createInfo.queueFamilyIndexCount = 2;
                    createInfo.pQueueFamilyIndices = queueFamilyIndices;
                }
                else {
                    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                }

                createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
                createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

                createInfo.presentMode = presentMode;
                createInfo.clipped = VK_TRUE;

                createInfo.oldSwapchain = VK_NULL_HANDLE;

                VkResult swapChainResult = vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapChain);
                if (swapChainResult != VK_SUCCESS) {
                    throw std::runtime_error("failed to create swap chain!");
                }

                vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, nullptr);
                //_swapChainImages.resize(imageCount);
                std::vector<VkImage> swapChainImages(imageCount);
                vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, swapChainImages.data());

                //_swapChainData.resize(imageCount);
                for (int i = 0; i < SWAPCHAIN_MAX_SIZE; i++) {
                    if (i < imageCount) {
                        AllocatedImage allocImage{};
                        allocImage.image = swapChainImages[i];
                        allocImage.useMipMap = false;

                        SwapChainData swapChainData{};
                        swapChainData.allocatedImage = allocImage;
                        _swapChainData[i] = swapChainData;
                    }
                    _swapChainData[i].wasInitialized = i < imageCount;
                }

                _swapChainSize = imageCount;
                _swapChainImageFormat = surfaceFormat.format;
                _swapChainExtent = extent;
            }

            void VKEngine::createSwapChainImageViews() {
                //for (size_t i = 0; i < _swapChainData.size(); i++) {
                for (size_t i = 0; i < _swapChainSize; i++) {
                    //_swapChainImageViews[i] = createImageView(_swapChainImages[i], _swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
                    createImageView(_swapChainData[i].allocatedImage, _swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
                }
            }

            void VKEngine::cleanupSwapChain() {
                for (auto swapChainData : _swapChainData) {
                    if(swapChainData.wasInitialized)
                        swapChainData.DestroySwapChainData(_device);
                }

                vkDestroySwapchainKHR(_device, _swapChain, nullptr);
            }
#pragma endregion

#pragma region Buffers
            VkDeviceSize VKEngine::getAlignment(VkDeviceSize bufferSize, VkDeviceSize minBufferAlignment) {
                if (minBufferAlignment > 0) {
                    return (bufferSize + minBufferAlignment - 1) & ~(minBufferAlignment - 1);
                }
                return bufferSize;
            }

            void VKEngine::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer, VmaAllocation& allocation, VmaAllocationInfo& allocationInfo) {
                VkBufferCreateInfo bufferInfo{};
                bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                bufferInfo.size = size;
                bufferInfo.usage = usage;
                bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

                VmaAllocationCreateInfo vbAllocCreateInfo = {};
                vbAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
                vbAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

                VkResult bufferCreateResult = vmaCreateBuffer(_allocator, &bufferInfo, &vbAllocCreateInfo, &buffer, &allocation, &allocationInfo);
                if (bufferCreateResult != VK_SUCCESS) {
                    throw std::runtime_error("failed to create buffer!");
                }

                /*if (vkCreateBuffer(_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create buffer!");
                }

                VkMemoryRequirements memRequirements;
                vkGetBufferMemoryRequirements(_device, buffer, &memRequirements);

                VkMemoryAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.allocationSize = memRequirements.size;
                allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

                if (vkAllocateMemory(_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
                    throw std::runtime_error("failed to allocate buffer memory!");
                }

                vkBindBufferMemory(_device, buffer, bufferMemory, 0);*/
            }

            void VKEngine::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, BufferObject& bufferObject) {
                createBuffer(size, usage, bufferObject.buffer, bufferObject.allocation, bufferObject.allocationInfo);
            }

            uint32_t VKEngine::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
                VkPhysicalDeviceMemoryProperties memProperties;
                vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);

                for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
                    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                        return i;
                    }
                }

                throw std::runtime_error("failed to find suitable memory type!");
            }

            void VKEngine::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
                VkCommandBuffer commandBuffer = beginSingleTimeCommands();

                VkBufferCopy copyRegion{};
                copyRegion.size = size;
                vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

                endSingleTimeCommands(commandBuffer);
            }

            void VKEngine::createAndFillBuffer(VkDeviceSize size, VkBufferUsageFlags usage, const void* data, BufferObject& bufferObject) {
                /*VkBuffer stagingBuffer;
                VmaAllocation stagingAllocation;
                VmaAllocationInfo stagingAllocationInfo;*/
                BufferObject stagingBufferObject;

                createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBufferObject);

                memcpy(stagingBufferObject.allocationInfo.pMappedData, data, (size_t)size);
                //vmaFreeMemory(_allocator, stagingBufferObject.allocation);

                createBuffer(size, usage, bufferObject);

                copyBuffer(stagingBufferObject.buffer, bufferObject.buffer, size);
                stagingBufferObject.DestroyBufferObject(_allocator);
                //vmaDestroyBuffer(_allocator, stagingBufferObject, stagingAllocation);
            }

            VkCommandBuffer VKEngine::beginSingleTimeCommands() {
                VkCommandBufferAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                allocInfo.commandPool = _transferCommandPool;
                allocInfo.commandBufferCount = 1;

                VkCommandBuffer commandBuffer;
                vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer);

                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

                vkBeginCommandBuffer(commandBuffer, &beginInfo);

                return commandBuffer;
            }

            void VKEngine::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
                vkEndCommandBuffer(commandBuffer);

                VkSubmitInfo submitInfo{};
                submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &commandBuffer;

                vkQueueSubmit(_transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
                vkQueueWaitIdle(_transferQueue);

                vkFreeCommandBuffers(_device, _transferCommandPool, 1, &commandBuffer);
            }
#pragma endregion

#pragma region Rendering
            void VKEngine::createRenderPass() {
                VkAttachmentDescription colorAttachment{};
                colorAttachment.format = _swapChainImageFormat;
                colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
                colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

                VkAttachmentReference colorAttachmentRef{};
                colorAttachmentRef.attachment = 0;
                colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                VkAttachmentDescription depthAttachment{};
                depthAttachment.format = findDepthFormat();
                depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
                depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

                VkAttachmentReference depthAttachmentRef{};
                depthAttachmentRef.attachment = 1;
                depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

                VkSubpassDescription subpass{};
                subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                subpass.colorAttachmentCount = 1;
                subpass.pColorAttachments = &colorAttachmentRef;
                subpass.pDepthStencilAttachment = &depthAttachmentRef;

                VkSubpassDependency dependency{};
                dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
                dependency.dstSubpass = 0;
                dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
                dependency.srcAccessMask = 0;
                dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
                dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

                std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
                VkRenderPassCreateInfo renderPassInfo{};
                renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
                renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
                renderPassInfo.pAttachments = attachments.data();
                renderPassInfo.subpassCount = 1;
                renderPassInfo.pSubpasses = &subpass;
                renderPassInfo.dependencyCount = 1;
                renderPassInfo.pDependencies = &dependency;

                if (vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderPass) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create render pass!");
                }
            }

            void VKEngine::createGraphicsPipeline() {
                //auto vertShaderCode = readFile("src/shaders/vert.spv");
                //auto fragShaderCode = readFile("src/shaders/frag.spv");
                std::string vertFile(SOURCE_PATH), fragFile(SOURCE_PATH);
                vertFile += "shaders/vert.spv";
                fragFile += "shaders/frag.spv";
                auto vertShaderCode = readFile(vertFile);
                auto fragShaderCode = readFile(fragFile);
                //shader module (code object?)
                VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
                VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
                //shaderStage
                //vertex 
                VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
                vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
                vertShaderStageInfo.module = vertShaderModule;
                vertShaderStageInfo.pName = "main";
                //fragment
                VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
                fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                fragShaderStageInfo.module = fragShaderModule;
                fragShaderStageInfo.pName = "main";

                VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

                //vertex input (like from a model)
                auto bindingDescription = Vertex::getBindingDescription();
                auto attributeDescriptions = Vertex::getAttributeDescriptions();

                VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
                vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                vertexInputInfo.vertexBindingDescriptionCount = 1;
                vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; // Optional
                vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());;
                vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); // Optional

                //Input assembler
                VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
                inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                inputAssembly.primitiveRestartEnable = VK_FALSE;

                //view port and scissors (almost like viewport clipping)
                VkViewport viewport{};
                viewport.x = 0.0f;
                viewport.y = 0.0f;
                viewport.width = (float)_swapChainExtent.width;
                viewport.height = (float)_swapChainExtent.height;
                viewport.minDepth = 0.0f;
                viewport.maxDepth = 1.0f;

                VkRect2D scissor{};
                scissor.offset = { 0, 0 };
                scissor.extent = _swapChainExtent;

                std::vector<VkDynamicState> dynamicStates = {
                    VK_DYNAMIC_STATE_VIEWPORT,
                    VK_DYNAMIC_STATE_SCISSOR
                };

                //viewports and scissors usually are dynamic
                VkPipelineDynamicStateCreateInfo dynamicState{};
                dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
                dynamicState.pDynamicStates = dynamicStates.data();

                VkPipelineViewportStateCreateInfo viewportState{};
                viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
                viewportState.viewportCount = 1;
                viewportState.pViewports = &viewport;
                viewportState.scissorCount = 1;
                viewportState.pScissors = &scissor;

                //rasterizer
                VkPipelineRasterizationStateCreateInfo rasterizer{};
                rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                rasterizer.depthClampEnable = VK_FALSE;
                rasterizer.rasterizerDiscardEnable = VK_FALSE;
                rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
                rasterizer.lineWidth = 1.0f;
                rasterizer.cullMode = VK_CULL_MODE_NONE;
                rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
                rasterizer.depthBiasEnable = VK_FALSE;
                rasterizer.depthBiasConstantFactor = 0.0f; // Optional
                rasterizer.depthBiasClamp = 0.0f; // Optional
                rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

                //multisapling (can be used for more efficient AA stuff)
                VkPipelineMultisampleStateCreateInfo multisampling{};
                multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
                multisampling.sampleShadingEnable = VK_FALSE;
                multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
                multisampling.minSampleShading = 1.0f; // Optional
                multisampling.pSampleMask = nullptr; // Optional
                multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
                multisampling.alphaToOneEnable = VK_FALSE; // Optional

                //depth and stencil testing (TBD)

                //Color blending (for each frame buffer and for global color blending)
                VkPipelineColorBlendAttachmentState colorBlendAttachment{};
                colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
                colorBlendAttachment.blendEnable = VK_FALSE;
                colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
                colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
                colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
                colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
                colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
                colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

                VkPipelineColorBlendStateCreateInfo colorBlending{};
                colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                colorBlending.logicOpEnable = VK_FALSE;
                colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
                colorBlending.attachmentCount = 1;
                colorBlending.pAttachments = &colorBlendAttachment;
                colorBlending.blendConstants[0] = 0.0f; // Optional
                colorBlending.blendConstants[1] = 0.0f; // Optional
                colorBlending.blendConstants[2] = 0.0f; // Optional
                colorBlending.blendConstants[3] = 0.0f; // Optional

                //VkDescriptorSetLayout descLayouts[] = { _descriptorSetCameraLayout, _descriptorSetObjectLayout };
                //VkDescriptorSetLayout descLayouts[] = { _descriptorSetLayout };
                VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
                pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
                pipelineLayoutInfo.setLayoutCount = 1; // Optional
                pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;
                //pipelineLayoutInfo.pSetLayouts = descLayouts;
                pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
                pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

                if (vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create pipeline layout!");
                }

                //Depth Stencil
                VkPipelineDepthStencilStateCreateInfo depthStencil{};
                depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
                depthStencil.depthTestEnable = VK_TRUE;
                depthStencil.depthWriteEnable = VK_TRUE;
                depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
                depthStencil.depthBoundsTestEnable = VK_FALSE;
                depthStencil.minDepthBounds = 0.0f; // Optional
                depthStencil.maxDepthBounds = 1.0f; // Optional
                depthStencil.stencilTestEnable = VK_FALSE;
                depthStencil.front = {}; // Optional
                depthStencil.back = {}; // Optional

                VkGraphicsPipelineCreateInfo pipelineInfo{};
                pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
                pipelineInfo.stageCount = 2;
                pipelineInfo.pStages = shaderStages;

                pipelineInfo.pVertexInputState = &vertexInputInfo;
                pipelineInfo.pInputAssemblyState = &inputAssembly;
                pipelineInfo.pViewportState = &viewportState;
                pipelineInfo.pRasterizationState = &rasterizer;
                pipelineInfo.pMultisampleState = &multisampling;
                pipelineInfo.pDepthStencilState = &depthStencil; // Optional
                pipelineInfo.pColorBlendState = &colorBlending;
                pipelineInfo.pDynamicState = &dynamicState;
                pipelineInfo.layout = _pipelineLayout;

                pipelineInfo.renderPass = _renderPass;
                pipelineInfo.subpass = 0;

                pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
                pipelineInfo.basePipelineIndex = -1; // Optional

                if (vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipeline) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create graphics pipeline!");
                }

                vkDestroyShaderModule(_device, fragShaderModule, nullptr);
                vkDestroyShaderModule(_device, vertShaderModule, nullptr);
            }

            VkShaderModule VKEngine::createShaderModule(const std::vector<char>& code) {
                VkShaderModuleCreateInfo createInfo{};
                createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                createInfo.codeSize = code.size();
                createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

                VkShaderModule shaderModule;
                if (vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create shader module!");
                }

                return shaderModule;
            }

            void VKEngine::createFramebuffers() {
                //_swapChainFramebuffers.resize(_swapChainImageViews.size());
                //_swapChainFramebuffers = std::make_unique<VkFramebuffer[]>(_swapChainImageViews.size());

                //for (size_t i = 0; i < _swapChainData.size(); i++) {
                for (size_t i = 0; i < _swapChainSize; i++) {
                    std::array<VkImageView, 2> attachments = {
                        //_swapChainImageViews[i],
                        _swapChainData[i].allocatedImage.imageView,
                        _depthTextureImage.imageView
                    };

                    VkFramebufferCreateInfo framebufferInfo{};
                    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                    framebufferInfo.renderPass = _renderPass;
                    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
                    framebufferInfo.pAttachments = attachments.data();
                    framebufferInfo.width = _swapChainExtent.width;
                    framebufferInfo.height = _swapChainExtent.height;
                    framebufferInfo.layers = 1;

                    //if (vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &_swapChainFramebuffers[i]) != VK_SUCCESS) {
                    if (vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &_swapChainData[i].frameBuffer) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create framebuffer!");
                    }
                }
            }

            void VKEngine::createCommandPool() {
                QueueFamilyIndices queueFamilyIndices = findQueueFamilies(_physicalDevice);

                VkCommandPoolCreateInfo poolInfo{};
                poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
                poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

                if (vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create command pool!");
                }

                poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
                poolInfo.queueFamilyIndex = queueFamilyIndices.transferFamily.value();

                if (vkCreateCommandPool(_device, &poolInfo, nullptr, &_transferCommandPool) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create transfer command pool!");
                }
            }

            void VKEngine::createCommandBuffer() {
                //commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

                VkCommandBufferAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocInfo.commandPool = _commandPool;
                allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

                for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    if (vkAllocateCommandBuffers(_device, &allocInfo, &_frames[i].commandBuffer) != VK_SUCCESS) {
                        throw std::runtime_error("failed to allocate command buffers!");
                    }
                }
            }

            void VKEngine::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, std::vector<RenderObject>* renderObjects) {
                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

                if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
                    throw std::runtime_error("failed to begin recording command buffer!");
                }

                std::array<VkClearValue, 2> clearValues{};
                clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
                clearValues[1].depthStencil = { 1.0f, 0 };

                VkRenderPassBeginInfo renderPassInfo{};
                renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                renderPassInfo.renderPass = _renderPass;
                //renderPassInfo.framebuffer = _swapChainFramebuffers[imageIndex];
                renderPassInfo.framebuffer = _swapChainData[imageIndex].frameBuffer;
                renderPassInfo.renderArea.offset = { 0, 0 };
                renderPassInfo.renderArea.extent = _swapChainExtent;

                renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
                renderPassInfo.pClearValues = clearValues.data();

                vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);

                VkViewport viewport{};
                viewport.x = 0.0f;
                viewport.y = 0.0f;
                viewport.width = (float)_swapChainExtent.width;
                viewport.height = (float)_swapChainExtent.height;
                viewport.minDepth = 0.0f;
                viewport.maxDepth = 1.0f;
                vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

                VkRect2D scissor{};
                scissor.offset = { 0, 0 };
                scissor.extent = _swapChainExtent;
                vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

                VkBuffer vertexBuffers[] = { _vertexBufferObject.buffer };
                VkDeviceSize offsets[] = { 0 };
                vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

                vkCmdBindIndexBuffer(commandBuffer, _indexBufferObject.buffer, 0, VK_INDEX_TYPE_UINT32);

                static auto startTime = std::chrono::high_resolution_clock::now();


                for (unsigned int i = 0; i < (*renderObjects).size(); i++) {

                    uint32_t uniform_offset = static_cast<uint32_t>(_renderOjbectsDynamicUniformBufferMemory[_currentFrame].alignmentSize) * i;
                    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSets[_currentFrame], 1, &uniform_offset);

                    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
                }

                vkCmdEndRenderPass(commandBuffer);

                if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
                    throw std::runtime_error("failed to record command buffer!");
                }
            }

#pragma endregion

#pragma region Mesh buffers
            void VKEngine::createVertexBuffer() {
                VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
                createAndFillBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertices.data(), _vertexBufferObject);

                //VkBuffer stagingBuffer;
                //VmaAllocation stagingAllocation;
                //VmaAllocationInfo stagingAllocation;
                ////VkDeviceMemory stagingBufferMemory;
                //createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingAllocation);

                //void* data;
                ////vkMapMemory(_device, stagingBufferMemory, 0, bufferSize, 0, &data);
                //memcpy(stagingAllocation->GetMappedData(), vertices.data(), (size_t)bufferSize);
                //vmaFreeMemory(_allocator, stagingAllocation);
                ////vkUnmapMemory(_device, stagingBufferMemory);

                //createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _vertexBuffer, stagingAllocation);

                //copyBuffer(stagingBuffer, _vertexBuffer, bufferSize);
                ////vkDestroyBuffer(_device, stagingBuffer, nullptr);
                ////vkFreeMemory(_device, stagingBufferMemory, nullptr);
                //vmaDestroyBuffer(_allocator, stagingBuffer, stagingAllocation);
            }

            void VKEngine::createIndexBuffer() {
                VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
                createAndFillBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indices.data(), _indexBufferObject);

                //VkBuffer stagingBuffer;
                ////VkDeviceMemory stagingBufferMemory;
                //VmaAllocation stagingAllocation;
                //createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingAllocation);

                //void* data;
                ////vkMapMemory(_device, stagingBufferMemory, 0, bufferSize, 0, &data);
                //memcpy(stagingAllocation->GetMappedData(), indices.data(), (size_t)bufferSize);
                ////vkUnmapMemory(_device, stagingBufferMemory);
                //vmaFreeMemory(_allocator, stagingAllocation);

                //createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _indexBuffer, stagingAllocation);

                //copyBuffer(stagingBuffer, _indexBuffer, bufferSize);
                ////vkDestroyBuffer(_device, stagingBuffer, nullptr);
                ////vkFreeMemory(_device, stagingBufferMemory, nullptr);
                //vmaDestroyBuffer(_allocator, stagingBuffer, stagingAllocation);
            }
#pragma endregion

#pragma region Descriptors
            void VKEngine::updateUniformBuffers(Camera camera, std::vector<RenderObject>* renderObjects) {

                CameraUniformBufferOjbect camBufferObject{};
                camBufferObject.view = glm::mat4(1);
                camBufferObject.proj = glm::mat4(4);

                //memcpy(_cameraUniformBufferMemory[_currentFrame].uniformBuffersMapped, &camBufferObject, _cameraUniformBufferMemory[_currentFrame].bufferSize);
                //dmemcpy(objectBufferPtr, (*renderObjects)[0].GetUniformBufferObject(), _renderOjbectsDynamicUniformBufferMemory[_currentFrame].alignmentSize);
                memcpy(_cameraUniformBufferMemory[_currentFrame].allocationInfo.pMappedData, camera.GetUniformBufferObject(), _cameraUniformBufferMemory[_currentFrame].allocationInfo.size);

                char* objectBufferPtr = static_cast<char*>(_renderOjbectsDynamicUniformBufferMemory[_currentFrame].allocationInfo.pMappedData);
                for (unsigned int i = 0; i < (*renderObjects).size(); i++) {
                    uint32_t uniform_offset = static_cast<uint32_t>(_renderOjbectsDynamicUniformBufferMemory[_currentFrame].alignmentSize) * i;
                    memcpy(objectBufferPtr + uniform_offset, (*renderObjects)[i].GetUniformBufferObject(), _renderOjbectsDynamicUniformBufferMemory[_currentFrame].alignmentSize);
                }
            }

            void VKEngine::createUniformBuffers() {
                size_t minUboAlignment = _physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
                VkDeviceSize cameraBufferSize = sizeof(CameraUniformBufferOjbect);
                VkDeviceSize renderObjectsAlignment = getAlignment(sizeof(RenderObjectUniformBufferOjbect), minUboAlignment);
                VkDeviceSize renderObjectsBufferSize = renderObjectsAlignment * MAX_INSTANCE_COUNT;

                for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    createBuffer(cameraBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, _cameraUniformBufferMemory[i]);
                    _cameraUniformBufferMemory[i].alignmentSize = cameraBufferSize;

                    createBuffer(cameraBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, _renderOjbectsDynamicUniformBufferMemory[i]);
                    _renderOjbectsDynamicUniformBufferMemory[i].alignmentSize = renderObjectsAlignment;
                }
            }

            void VKEngine::createDescriptorPool() {
                VkDescriptorPoolSize cameraPoolSize{};
                cameraPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                cameraPoolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

                VkDescriptorPoolSize texSamplerPoolSize{};
                texSamplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                texSamplerPoolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

                VkDescriptorPoolSize renderObjectPoolSize{};
                renderObjectPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                renderObjectPoolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

                VkDescriptorPoolSize poolSizes[] = { cameraPoolSize , texSamplerPoolSize, renderObjectPoolSize };

                VkDescriptorPoolCreateInfo poolInfo{};
                poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                poolInfo.poolSizeCount = 3;
                poolInfo.pPoolSizes = poolSizes;
                poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

                if (vkCreateDescriptorPool(_device, &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create descriptor pool!");
                }
            }

            void VKEngine::createDescriptorSetLayout() {
                VkDescriptorSetLayoutBinding cameraUboLayoutBinding{};
                cameraUboLayoutBinding.binding = 0;
                cameraUboLayoutBinding.descriptorCount = 1;
                cameraUboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                cameraUboLayoutBinding.pImmutableSamplers = nullptr;
                cameraUboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                VkDescriptorSetLayoutBinding textureUboLayoutBinding{};
                textureUboLayoutBinding.binding = 1;
                textureUboLayoutBinding.descriptorCount = 1;
                textureUboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                textureUboLayoutBinding.pImmutableSamplers = nullptr;
                textureUboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                VkDescriptorSetLayoutBinding modelUboLayoutBinding{};
                modelUboLayoutBinding.binding = 2;
                modelUboLayoutBinding.descriptorCount = 1;
                modelUboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                modelUboLayoutBinding.pImmutableSamplers = nullptr;
                modelUboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                std::vector<VkDescriptorSetLayoutBinding> set_layout_bindings =
                {
                    cameraUboLayoutBinding,
                    textureUboLayoutBinding,
                    modelUboLayoutBinding,
                };

                VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
                layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                layoutCreateInfo.bindingCount = set_layout_bindings.size();
                layoutCreateInfo.pBindings = set_layout_bindings.data();

                if (vkCreateDescriptorSetLayout(_device, &layoutCreateInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create descriptor set camera layout!");
                }
            }

            void VKEngine::createDescriptorSets() {
                std::vector<VkDescriptorSetLayout> camLayouts(MAX_FRAMES_IN_FLIGHT, _descriptorSetLayout);
                VkDescriptorSetAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = _descriptorPool;
                allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
                allocInfo.pSetLayouts = camLayouts.data();

                if (vkAllocateDescriptorSets(_device, &allocInfo, _descriptorSets) != VK_SUCCESS) {
                    throw std::runtime_error("failed to allocate descriptor sets!");
                }

                for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    VkDescriptorBufferInfo cameraBufferInfo{};
                    cameraBufferInfo.buffer = _cameraUniformBufferMemory[i].buffer;
                    cameraBufferInfo.range = _cameraUniformBufferMemory[i].allocationInfo.size;
                    cameraBufferInfo.offset = 0;

                    VkWriteDescriptorSet cameraDescriptorWrite{};
                    cameraDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    cameraDescriptorWrite.dstSet = _descriptorSets[i];
                    cameraDescriptorWrite.dstBinding = 0;
                    cameraDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    cameraDescriptorWrite.descriptorCount = 1;
                    cameraDescriptorWrite.pBufferInfo = &cameraBufferInfo;

                    //Image descriptor write
                    VkDescriptorImageInfo imageInfo{};
                    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    imageInfo.imageView = _modelTextureImage.imageView;
                    imageInfo.sampler = _textureSampler;

                    VkWriteDescriptorSet imageDescriptorWrite{};
                    imageDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    imageDescriptorWrite.dstSet = _descriptorSets[i];
                    imageDescriptorWrite.dstBinding = 1;
                    imageDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    imageDescriptorWrite.descriptorCount = 1;
                    imageDescriptorWrite.pImageInfo = &imageInfo;

                    VkDescriptorBufferInfo objectBufferInfo{};
                    objectBufferInfo.buffer = _renderOjbectsDynamicUniformBufferMemory[i].buffer;
                    objectBufferInfo.range = _renderOjbectsDynamicUniformBufferMemory[i].allocationInfo.size;
                    objectBufferInfo.offset = 0;

                    VkWriteDescriptorSet objectDescriptorWrite{};
                    objectDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    objectDescriptorWrite.dstSet = _descriptorSets[i];
                    objectDescriptorWrite.dstBinding = 2;
                    objectDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                    objectDescriptorWrite.descriptorCount = 1;
                    objectDescriptorWrite.pBufferInfo = &objectBufferInfo;

                    VkWriteDescriptorSet descriptorWriteSets[] = { cameraDescriptorWrite , imageDescriptorWrite, objectDescriptorWrite };
                    vkUpdateDescriptorSets(_device, 3, descriptorWriteSets, 0, nullptr);
                }
            }
#pragma endregion

#pragma region Textures
            void VKEngine::createTextureImage() {
                int texWidth, texHeight, texChannels;
                //stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
                //stbi_uc* pixels = stbi_load("resources/textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
                stbi_uc* pixels = stbi_load((RESOURCES_PATH + TEXTURE_PATH).c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
                VkDeviceSize imageSize = texWidth * texHeight * 4;

                if (!pixels) {
                    throw std::runtime_error("failed to load texture image!");
                }

                VkBuffer stagingBuffer;
                VmaAllocation stagingAllocation;
                VmaAllocationInfo stagingAllocationInfo;
                createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBuffer, stagingAllocation, stagingAllocationInfo);

                //void* data;
                //vkMapMemory(_device, stagingBufferMemory, 0, imageSize, 0, &data);
                //vkUnmapMemory(_device, stagingBufferMemory);
                memcpy(stagingAllocationInfo.pMappedData, pixels, static_cast<size_t>(imageSize));

                stbi_image_free(pixels);

                _modelTextureImage.useMipMap = true;
                _modelTextureImage.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
                //_modelTextureImage.mipLevels = 1;

                createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, _modelTextureImage);
                //createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, _modelTextureImage);
                
                transitionImageLayout(_modelTextureImage.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, _modelTextureImage.mipLevels);
                copyBufferToImage(stagingBuffer, _modelTextureImage.image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

                transitionImageLayout(_modelTextureImage.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, _modelTextureImage.mipLevels);

                vmaDestroyBuffer(_allocator, stagingBuffer, stagingAllocation);



                //createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _modelTextureImage);

                //transitionImageLayout(_modelTextureImage.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
                //copyBufferToImage(stagingBuffer, _modelTextureImage.image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

                //transitionImageLayout(_modelTextureImage.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

                generateMipmaps(_modelTextureImage.image, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, _modelTextureImage.mipLevels);
            }

            void  VKEngine::generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {

                VkFormatProperties formatProperties;
                vkGetPhysicalDeviceFormatProperties(_physicalDevice, imageFormat, &formatProperties);
                if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
                    throw std::runtime_error("texture image format does not support linear blitting!");
                }

                VkCommandBuffer commandBuffer = beginSingleTimeCommands();

                VkImageMemoryBarrier barrier{};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.image = image;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = 1;
                barrier.subresourceRange.levelCount = 1;

                int32_t mipWidth = texWidth;
                int32_t mipHeight = texHeight;

                for (uint32_t i = 1; i < mipLevels; i++) {
                    barrier.subresourceRange.baseMipLevel = i - 1;
                    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

                    vkCmdPipelineBarrier(commandBuffer,
                        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                        0, nullptr,
                        0, nullptr,
                        1, &barrier);

                    VkImageBlit blit{};
                    blit.srcOffsets[0] = { 0, 0, 0 };
                    blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
                    blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    blit.srcSubresource.mipLevel = i - 1;
                    blit.srcSubresource.baseArrayLayer = 0;
                    blit.srcSubresource.layerCount = 1;
                    blit.dstOffsets[0] = { 0, 0, 0 };
                    blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
                    blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    blit.dstSubresource.mipLevel = i;
                    blit.dstSubresource.baseArrayLayer = 0;
                    blit.dstSubresource.layerCount = 1;

                    vkCmdBlitImage(commandBuffer,
                        image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        1, &blit,
                        VK_FILTER_LINEAR);

                    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                    vkCmdPipelineBarrier(commandBuffer,
                        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                        0, nullptr,
                        0, nullptr,
                        1, &barrier);

                    if (mipWidth > 1) mipWidth /= 2;
                    if (mipHeight > 1) mipHeight /= 2;
                }

                barrier.subresourceRange.baseMipLevel = mipLevels - 1;
                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                vkCmdPipelineBarrier(commandBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier);

                endSingleTimeCommands(commandBuffer);
            }

            void VKEngine::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, AllocatedImage& allocatedImage) {
                VkImageCreateInfo imageInfo{};
                imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                imageInfo.imageType = VK_IMAGE_TYPE_2D;
                imageInfo.extent.width = width;
                imageInfo.extent.height = height;
                imageInfo.extent.depth = 1;
                if (allocatedImage.useMipMap) {
                    imageInfo.mipLevels = allocatedImage.mipLevels;
                }
                else {
                    imageInfo.mipLevels = 1;
                }
                imageInfo.arrayLayers = 1;
                imageInfo.format = format;
                imageInfo.tiling = tiling;
                imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                imageInfo.usage = usage;
                imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

                VmaAllocationCreateInfo allocinfo = {};
                allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
                allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

                VkResult createImageResult = vmaCreateImage(_allocator, &imageInfo, &allocinfo, &allocatedImage.image, &allocatedImage.allocation, nullptr);
                if (createImageResult != VK_SUCCESS) {
                    throw std::runtime_error("failed to create image!");
                }
            }

            void VKEngine::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {
                VkCommandBuffer commandBuffer = beginSingleTimeCommands();

                VkImageMemoryBarrier barrier{};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.oldLayout = oldLayout;
                barrier.newLayout = newLayout;

                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

                barrier.image = image;
                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = 1;

                if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
                    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

                    if (hasStencilComponent(format)) {
                        barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
                    }
                }
                else {
                    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                }

                VkPipelineStageFlags sourceStage;
                VkPipelineStageFlags destinationStage;

                if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                    barrier.srcAccessMask = 0;
                    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                }
                else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                }
                else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
                    barrier.srcAccessMask = 0;
                    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

                    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                    destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
                }
                else {
                    throw std::invalid_argument("unsupported layout transition!");
                }

                vkCmdPipelineBarrier(
                    commandBuffer,
                    sourceStage, destinationStage,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier
                );

                endSingleTimeCommands(commandBuffer);
            }

            void VKEngine::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
                VkCommandBuffer commandBuffer = beginSingleTimeCommands();

                VkBufferImageCopy region{};
                region.bufferOffset = 0;
                region.bufferRowLength = 0;
                region.bufferImageHeight = 0;

                region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                region.imageSubresource.mipLevel = 0;
                region.imageSubresource.baseArrayLayer = 0;
                region.imageSubresource.layerCount = 1;

                region.imageOffset = { 0, 0, 0 };
                region.imageExtent = {
                    width,
                    height,
                    1
                };

                vkCmdCopyBufferToImage(
                    commandBuffer,
                    buffer,
                    image,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1,
                    &region
                );

                endSingleTimeCommands(commandBuffer);
            }

            void VKEngine::createImageView(AllocatedImage& allocatedImage, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
                VkImageViewCreateInfo viewInfo{};
                viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                viewInfo.image = allocatedImage.image;
                viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                viewInfo.format = format;
                viewInfo.subresourceRange.aspectMask = aspectFlags;
                viewInfo.subresourceRange.baseMipLevel = 0;
                viewInfo.subresourceRange.levelCount = mipLevels;
                viewInfo.subresourceRange.baseArrayLayer = 0;
                viewInfo.subresourceRange.layerCount = 1;

                if (vkCreateImageView(_device, &viewInfo, nullptr, &allocatedImage.imageView) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create texture image view!");
                }
            }

            void VKEngine::createTextureImageView() {
                uint32_t mipLevels = 1;
                if (_modelTextureImage.useMipMap) {
                    mipLevels = _modelTextureImage.mipLevels;
                }
                createImageView(_modelTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
            }

            void VKEngine::createTextureSampler() {
                VkSamplerCreateInfo samplerInfo{};
                samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
                samplerInfo.magFilter = VK_FILTER_LINEAR;
                samplerInfo.minFilter = VK_FILTER_LINEAR;
                samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerInfo.anisotropyEnable = VK_TRUE;
                samplerInfo.maxAnisotropy = _physicalDeviceProperties.limits.maxSamplerAnisotropy;
                samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
                samplerInfo.unnormalizedCoordinates = VK_FALSE;
                samplerInfo.compareEnable = VK_FALSE;
                samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

                samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
                samplerInfo.mipLodBias = 0.0f;
                samplerInfo.minLod = 0.0f;
                // samplerInfo.minLod = static_cast<float>(_modelTextureImage.mipLevels);
                //samplerInfo.maxLod = 0.0f;
                samplerInfo.maxLod = static_cast<float>(_modelTextureImage.mipLevels);

                if (vkCreateSampler(_device, &samplerInfo, nullptr, &_textureSampler) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create texture sampler!");
                }
            }

            void VKEngine::createDepthResources() {
                VkFormat depthFormat = findDepthFormat();

                _depthTextureImage.useMipMap = false;
                createImage(_swapChainExtent.width, _swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, _depthTextureImage);
                createImageView(_depthTextureImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

                transitionImageLayout(_depthTextureImage.image, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
            }

            VkFormat VKEngine::findDepthFormat() {
                return findSupportedFormat(
                    { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
                );
            }

            bool VKEngine::hasStencilComponent(VkFormat format) {
                return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
            }

            VkFormat VKEngine::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
                for (VkFormat format : candidates) {
                    VkFormatProperties props;
                    vkGetPhysicalDeviceFormatProperties(_physicalDevice, format, &props);

                    if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                        return format;
                    }
                    else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                        return format;
                    }
                }

                throw std::runtime_error("failed to find supported format!");
            }
#pragma endregion

#pragma region Model
            void VKEngine::loadModel() {
                tinyobj::attrib_t attrib;
                std::vector<tinyobj::shape_t> shapes;
                std::vector<tinyobj::material_t> materials;
                std::string warn, err;

                if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, (RESOURCES_PATH + MODEL_PATH).c_str())) {
                    throw std::runtime_error(err);
                }

                vertices.clear();
                indices.clear();

                std::unordered_map<Vertex, uint32_t> uniqueVertices{};

                glm::mat4 objMat = glm::mat4(1.0);
                objMat = glm::rotate(objMat, glm::radians(-90.0f), glm::vec3(0.0, 0.0, 1.0));
                objMat = glm::rotate(objMat, glm::radians(-90.0f), glm::vec3(0.0, 1.0, 0.0));

                for (const auto& shape : shapes) {
                    for (const auto& index : shape.mesh.indices) {
                        Vertex vertex{};

                        vertex.pos = {
                            attrib.vertices[3 * index.vertex_index + 0],
                            attrib.vertices[3 * index.vertex_index + 1],
                            attrib.vertices[3 * index.vertex_index + 2]
                        };

                        vertex.pos = objMat * glm::vec4(vertex.pos, 1.0);

                        vertex.texCoord = {
                            attrib.texcoords[2 * index.texcoord_index + 0],
                            1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                        };

                        vertex.color = { 1.0f, 1.0f, 1.0f };

                        if (uniqueVertices.count(vertex) == 0) {
                            uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                            vertices.push_back(vertex);
                        }

                        indices.push_back(uniqueVertices[vertex]);
                    }
                }
            }
#pragma endregion

#pragma region Syncing
            void VKEngine::createSyncObjects() {
                /*imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
                renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
                inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);*/

                VkSemaphoreCreateInfo semaphoreInfo{};
                semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

                VkFenceCreateInfo fenceInfo{};
                fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

                for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    if (vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_frames[i].presentSemaphore) != VK_SUCCESS ||
                        vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_frames[i].renderSemaphore) != VK_SUCCESS ||
                        vkCreateFence(_device, &fenceInfo, nullptr, &_frames[i].renderFence) != VK_SUCCESS) {

                        throw std::runtime_error("failed to create synchronization objects for a frame!");
                    }
                }
            }
#pragma endregion

#pragma region Getters
            FrameData& VKEngine::GetCurrentFrameData() {
                return _frames[_currentFrame];
            }
#pragma endregion

    }
}

