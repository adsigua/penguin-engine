#pragma once

#include <graphics/graphics_types.h>
#include "vk_includes.h"
#include "vk_vma_usage.h"

#include <span>
#include <vector>
#include <optional>

namespace penguin_engine {
namespace graphics {
namespace vulkan {

    struct FrameData {
        VkSemaphore acquireSwapchainSemaphore;
        //VkSemaphore renderSemaphore;
        VkFence renderFence;

        bool syncingInitialized = false;

        VkCommandPool commandPool;
        VkCommandBuffer commandBuffer;

        bool commandsInitialized = false;

        //DeletionQueue frameDeletionQueue{};

        void destroy_frame_data(VkDevice device) {
            if (syncingInitialized) {
                vkDestroySemaphore(device, acquireSwapchainSemaphore, nullptr);
                //vkDestroySemaphore(device, renderSemaphore, nullptr);
                vkDestroyFence(device, renderFence, nullptr);
            }
            if (commandsInitialized) {
                vkDestroyCommandPool(device, commandPool, nullptr);
            }
        }
    };

    struct SwapChainData {
        VkImage image;
        VkImageView imageView;
        VkFramebuffer frameBuffer;
        VkSemaphore submitSemaphore;
        //VkSemaphore semaphore;
        bool frameBufferInitialized = false;

        void destroy_swapchain_data(VkDevice device) {
            if (frameBufferInitialized) {
                vkDestroyFramebuffer(device, frameBuffer, nullptr);
            }
            vkDestroyImageView(device, imageView, nullptr);
            vkDestroySemaphore(device, submitSemaphore, nullptr);
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

    struct AllocatedBuffer {
        VkBuffer buffer;
        VmaAllocation allocation;
        VmaAllocationInfo info;
        VkDeviceSize alignmentSize;

        void destroy_allocated_buffer(VmaAllocator allocator) {
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

        VkExtent3D imageExtent;
        VkFormat imageFormat;

        bool useMipMap;
        bool imageViewInitialized;
        uint32_t mipLevels;

        void destroy_allocated_image(VkDevice device, VmaAllocator allocator) {
            if (imageViewInitialized) {
                vkDestroyImageView(device, imageView, nullptr);
            }
            vmaDestroyImage(allocator, image, allocation);
        }
    };

    struct DrawImageData {
        AllocatedImage allocatedImage;
        VkDescriptorSet drawImageDescriptors;
        VkDescriptorSetLayout drawImageDescriptorLayout;

        void destroy_draw_image(VkDevice device, VmaAllocator allocator) {
            allocatedImage.destroy_allocated_image(device, allocator);
            vkDestroyDescriptorSetLayout(device, drawImageDescriptorLayout, nullptr);
        }
    };

    struct ImgGUISyncData {
        VkFence fence;
        VkCommandBuffer commandBuffer;
        VkCommandPool commandPool;
        VkDescriptorPool descriptorPool;

        void destroy_sync_data(VkDevice device) {
            vkDestroyDescriptorPool(device, descriptorPool, nullptr);
            vkDestroyFence(device, fence, nullptr);
            vkDestroyCommandPool(device, commandPool, nullptr);
        }
    };
    
    // holds the resources needed for a mesh
    struct GPUMeshBuffers {
        AllocatedBuffer indexBuffer;
        AllocatedBuffer vertexBuffer;
        VkDeviceAddress vertexBufferAddress;

        void destroy_gpu_mesh_buffers(VmaAllocator allocator) {
            vertexBuffer.destroy_allocated_buffer(allocator);
            indexBuffer.destroy_allocated_buffer(allocator);
        }
    };

    // push constants for our mesh object draws
    struct GPUDrawPushConstants {
        glm::mat4 worldMatrix;
        VkDeviceAddress vertexBuffer;
    };

    struct RenderObjectUniformBufferOjbect {
        alignas(16) glm::mat4 model;
    };

    struct CameraUniformBufferOjbect {
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };

    struct ComputePushConstants {
        glm::vec4 data1;
        glm::vec4 data2;
        glm::vec4 data3;
        glm::vec4 data4;
    };

    struct ComputeEffect {
        const char* name;

        VkPipeline pipeline;
        VkPipelineLayout pipeline_layout;

        ComputePushConstants data;
    };

    struct GeoSurface {
        uint32_t startIndex;
        uint32_t count;
    };

    struct GPUMeshAsset {
        std::string name;
        std::vector<GeoSurface> surfaces;

        GPUMeshBuffers gpuMeshBuffers;
    };
}
}
}

