#pragma once
#ifndef PENGUIN_VK_TYPES
#define PENGUIN_VK_TYPES

#include <deque>

#include <optional>
#include <vector>

namespace PenguinEngine {
namespace Graphics {

    //struct DeletionQueue
    //{
    //    std::deque<std::function<void()>> deletors;

    //    void push_function(std::function<void()>&& function) {
    //        deletors.push_back(function);
    //    }

    //    void flush() {
    //        // reverse iterate the deletion queue to execute all the functions
    //        for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
    //            (*it)(); //call functors
    //        }

    //        deletors.clear();
    //    }
    //};

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

    //struct UniformBufferMemory {
    //    //VkBuffer uniformBuffer;
    //    //VmaAllocation allocation;
    //    //VmaAllocationInfo allocationInfo;
    //    //VkDeviceMemory deviceMemory;
    //    //void* uniformBuffersMapped;
    //    BufferObject bufferObject;
    //};

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
        VkFormat imageFormat;
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


}
}

#endif