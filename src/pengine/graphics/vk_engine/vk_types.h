#pragma once
#ifndef PENGUIN_VK_TYPES
#define PENGUIN_VK_TYPES

#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <span>
#include <array>
#include <functional>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "vk_vma_usage.h"

#include <pengine/core/core_includes.h>
#include <vk_mem_alloc.h>

namespace PenguinEngine {
namespace Graphics {
namespace Vulkan {

    struct FrameData {
        VkSemaphore presentSemaphore, renderSemaphore;
        VkFence renderFence;

        VkCommandPool commandPool;
        VkCommandBuffer commandBuffer;

        //DeletionQueue frameDeletionQueue{};

        void DestroyFrameData(VkDevice device) {
            vkDestroySemaphore(device, presentSemaphore, nullptr);
            vkDestroySemaphore(device, renderSemaphore, nullptr);
            vkDestroyFence(device, renderFence, nullptr);
        }
    };

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        std::optional<uint32_t> transferFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct BufferObject {
        VkBuffer buffer;
        VmaAllocation allocation;
        VmaAllocationInfo allocationInfo;
        VkDeviceSize alignmentSize;

        void DestroyBufferObject(VmaAllocator allocator) {
            //vkDestroyBuffer(device, uniformBuffer, nullptr);
            //vkFreeMemory(device, deviceMemory, nullptr);
            vmaDestroyBuffer(allocator, buffer, allocation);
        }
    };

    struct AllocatedImage {
        VkImage image;
        VkImageView imageView;

        VmaAllocation allocation;
        VmaAllocationInfo allocationInfo;

        VkExtent2D imageExtent;
        //VkFormat imageFormat;
        bool useMipMap;
        uint32_t mipLevels;

        void DestroyAllocatedImage(VkDevice device, VmaAllocator allocator) {
            vkDestroyImageView(device, imageView, nullptr);
            vmaDestroyImage(allocator, image, allocation);
        }
    };

    struct SwapChainData {
        AllocatedImage allocatedImage;
        VkFramebuffer frameBuffer;
        bool wasInitialized = false;

        void DestroySwapChainData(VkDevice device) {
            vkDestroyFramebuffer(device, frameBuffer, nullptr);
            vkDestroyImageView(device, allocatedImage.imageView, nullptr);
        }
    };

    struct RenderObjectUniformBufferOjbect {
        alignas(16) glm::mat4 model;
    };

    struct CameraUniformBufferOjbect {
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };
}
}
}

#endif