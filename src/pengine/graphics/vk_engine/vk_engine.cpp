#include "vk_engine.h"
#include "vk_initializers.h"
#include "vk_utils.h"
#include "vk_vma_usage.h"
#include "vk_mem_alloc.h"
#include "vk_loader.h"

//imgui
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_vulkan.h>

#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <array>
#include <functional>

namespace penguin_engine {
namespace graphics {
namespace vulkan {

    constexpr bool bUseValidationLayers = true;

    const std::string MODEL_PATH = "models/viking_room.obj";
    const std::string TEXTURE_PATH = "textures/viking_room.png";

    bool _framebufferResized = false;

    int _swapChainSize = 0;

    static constexpr uint32_t GetVulkanApiVersion()
    {
#if VMA_VULKAN_VERSION == 1004000
        return VK_API_VERSION_1_4;
#elif VMA_VULKAN_VERSION == 1003000
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

#pragma region Public
    void VKEngine::init_vk_engine(Window* window) {
        _window = window;
        init_vulkan();
    }

    void VKEngine::draw(Camera camera, std::vector<RenderObject>* renderObjects) {
        FrameData* currentFrameData = &get_current_framedata();
        VK_CHECK(vkWaitForFences(_logical_device, 1, &currentFrameData->renderFence, VK_TRUE, UINT64_MAX));
        VK_CHECK(vkResetFences(_logical_device, 1, &currentFrameData->renderFence));

        uint32_t swapchainImageIndex;
        VkResult result = vkAcquireNextImageKHR(_logical_device, _swapchain, UINT64_MAX, currentFrameData->acquireSwapchainSemaphore, VK_NULL_HANDLE, &swapchainImageIndex);
       
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreate_swapchain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        VkCommandBuffer cmd = currentFrameData->commandBuffer;

        VK_CHECK(vkResetCommandBuffer(cmd, /*VkCommandBufferResetFlagBits*/ 0));
        record_command_buffer(cmd, swapchainImageIndex, renderObjects);

       /* VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { currentFrameData.swapchainSemaphore };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &currentFrameData.commandBuffer;

        VkSemaphore signalSemaphores[] = { currentFrameData.renderSemaphore };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        VkResult queueSubmitResult = vkQueueSubmit(_graphics_queue, 1, &submitInfo, currentFrameData.renderFence);
        if (queueSubmitResult != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }*/

        VkCommandBufferSubmitInfo cmdinfo = vkinit::command_buffer_submit_info(cmd);

        VkSemaphoreSubmitInfo waitInfo = vkinit::semaphore_submit_info(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, currentFrameData->acquireSwapchainSemaphore);
        VkSemaphoreSubmitInfo signalInfo = vkinit::semaphore_submit_info(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, _swapchain_data[swapchainImageIndex].submitSemaphore);

        VkSubmitInfo2 submit = vkinit::submit_info(&cmdinfo, &signalInfo, &waitInfo);
        
        VK_CHECK(vkQueueSubmit2(_graphics_queue, 1, &submit, currentFrameData->renderFence));

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &_swapchain_data[swapchainImageIndex].submitSemaphore;

        VkSwapchainKHR swapChains[] = { _swapchain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &swapchainImageIndex;

        result = vkQueuePresentKHR(_present_queue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || _framebufferResized) {
            _framebufferResized = false;
            recreate_swapchain();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        _current_frame_number ++;
        _current_frame_index = _current_frame_number % MAX_FRAMES_IN_FLIGHT;
    }

    void VKEngine::cleanup() {
        if (!_isInitialized)
            return;

        for (auto& mesh : testMeshes) {
            mesh->gpuMeshBuffers.destroy_gpu_mesh_buffers(_vma_allocator);
        }

        vkDeviceWaitIdle(_logical_device);

        ImGui_ImplVulkan_Shutdown();
        _imgui_sync_data.destroy_sync_data(_logical_device);

        rectangle.destroy_gpu_mesh_buffers(_vma_allocator);

        //pipelines
        vkDestroyPipeline(_logical_device, _mesh_pipeline, nullptr);
        vkDestroyPipelineLayout(_logical_device, _mesh_pipeline_layout, nullptr);

        //vkDestroyPipeline(_logical_device, _triangle_pipeline, nullptr);
        //vkDestroyPipelineLayout(_logical_device, _triangle_pipeline_layout, nullptr);

        for (size_t i = 0; i < _backgroundEffects.size(); i++) {
            vkDestroyPipeline(_logical_device, _backgroundEffects[i].pipeline, nullptr);
        }
        vkDestroyPipelineLayout(_logical_device, _background_pipeline_layout, nullptr);

        _draw_image_data.destroy_draw_image(_logical_device, _vma_allocator);
        _depth_image.destroy_allocated_image(_logical_device, _vma_allocator);
        _descriptor_allocator.destroy_pool(_logical_device);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            _frames[i].destroy_frame_data(_logical_device);
        }

        destroy_swapchain();

        vkDestroySurfaceKHR(_instance, _surface, nullptr);
        vmaDestroyAllocator(_vma_allocator);

        vkDestroyDevice(_logical_device, nullptr);

        vkb::destroy_debug_utils_messenger(_instance, _debug_messenger);
        vkDestroyInstance(_instance, nullptr);

        //vkDestroyCommandPool(_device_logical, _commandPool, nullptr);
        //vkDestroyCommandPool(_device_logical, _transferCommandPool, nullptr);
        /*vkDestroyPipeline(_device, _graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);
        vkDestroyRenderPass(_device, _renderPass, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            _cameraUniformBufferMemory[i].DestroyBufferObject(_allocator);
            _renderOjbectsDynamicUniformBufferMemory[i].DestroyBufferObject(_allocator);
        }

        vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
        vkDestroySampler(_device, _textureSampler, nullptr);
        _modelTextureImage.DestroyAllocatedImage(_device, _allocator);
        _depthTextureImage.DestroyAllocatedImage(_device, _allocator);
        vkDestroyDescriptorSetLayout(_device, _descriptorSetLayout, nullptr);

        _vertexBufferObject.DestroyBufferObject(_allocator);
        _indexBufferObject.DestroyBufferObject(_allocator);

        */
    }

    ComputeEffect* VKEngine::get_current_background_fx() {
        return &_backgroundEffects[_currentBackgroundEffect];
    }

    int* VKEngine::get_background_fx_current_index() {
        return &_currentBackgroundEffect;
    }

    int VKEngine::get_background_fx_count() {
        return _backgroundEffects.size();
    }
    
    void VKEngine::recreate_swapchain() {
        destroy_swapchain();
        init_swapchain();
        /*int width = 0, height = 0;
        glfwGetFramebufferSize(_window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(_window, &width, &height);
            glfwWaitEvents();
        }
        */
        /*vkDeviceWaitIdle(_device_logical);

        destroy_swapchain();
        _depthTextureImage.DestroyAllocatedImage(_device, _allocator);

        createSwapChain();
        createSwapChainImageViews();
        createDepthResources();
        createFramebuffers();*/
    }

    void VKEngine::set_framebuffer_was_resized(bool wasResized) {
        _framebufferResized = wasResized;
    }

    float VKEngine::get_swapchain_aspect_ratio() {
        return _swapchain_extent.width / (float)_swapchain_extent.height;
    }
#pragma endregion
 
#pragma region initialize

    void VKEngine::init_vulkan()
    {
        vkb::InstanceBuilder builder;

        //make the vulkan instance, with basic debug features
        auto inst_ret = builder.set_app_name("Example Vulkan Application")
            .request_validation_layers(true)
            .use_default_debug_messenger()
            .require_api_version(1, 3, 0)
            .build();

        vkb::Instance vkb_inst = inst_ret.value();

        //grab the instance 
        _instance = vkb_inst.instance;
        _debug_messenger = vkb_inst.debug_messenger;

        SDL_Vulkan_CreateSurface(_window->getSDLWindow(), _instance, nullptr, &_surface);

        //vulkan 1.3 features
        VkPhysicalDeviceVulkan13Features features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
        features.dynamicRendering = true;
        features.synchronization2 = true;

        //vulkan 1.2 features
        VkPhysicalDeviceVulkan12Features features12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
        features12.bufferDeviceAddress = true;
        features12.descriptorIndexing = true;

        //use vkbootstrap to select a gpu. 
        //We want a gpu that can write to the SDL surface and supports vulkan 1.3 with the correct features
        vkb::PhysicalDeviceSelector selector{ vkb_inst };
        vkb::PhysicalDevice vkbPhysicalDevice = selector
            .set_minimum_version(1, 3)
            .set_required_features_13(features)
            .set_required_features_12(features12)
            .set_surface(_surface)
            .select()
            .value();


        //create the final vulkan device
        vkb::DeviceBuilder deviceBuilder{ vkbPhysicalDevice };

        vkb::Device vkbDevice = deviceBuilder.build().value();

        // Get the VkDevice handle used in the rest of a vulkan application
        _logical_device = vkbDevice.device;
        _physical_device = vkbPhysicalDevice.physical_device;
        _device_properties_physical = vkbPhysicalDevice.properties;

        init_vma();
        init_swapchain();
        init_draw_image();
        init_queue_families(vkbDevice);

        init_commands();
        init_sync_structs();
        init_descriptors();
        init_pipelines();

        init_default_data();

        init_imgui();

        _isInitialized = true;
    }

    void VKEngine::init_swapchain() {
        create_swapchain(_window->get_window_width(), _window->get_window_height());
    }

    void VKEngine::init_draw_image() {
        //draw image size will match the window
        VkExtent3D drawImageExtent = {
            _swapchain_extent.width,
            _swapchain_extent.height,
            1
        };

        //hardcoding the draw format to 32 bit float
        AllocatedImage *drawImage = &_draw_image_data.allocatedImage;
        drawImage->imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
        drawImage->imageExtent = drawImageExtent;

        VkImageUsageFlags drawImageUsages{};
        drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
        drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        VkImageCreateInfo rimg_info = vkinit::image_create_info(drawImage->imageFormat, drawImageUsages, drawImageExtent);

        //for the draw image, we want to allocate it from gpu local memory
        VmaAllocationCreateInfo rimg_allocinfo = {};
        rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        //allocate and create the image
        vmaCreateImage(_vma_allocator, &rimg_info, &rimg_allocinfo, &drawImage->image, &drawImage->allocation, nullptr);

        //build a image-view for the draw image to use for rendering
        VkImageViewCreateInfo rview_info = vkinit::imageview_create_info(drawImage->imageFormat, drawImage->image, VK_IMAGE_ASPECT_COLOR_BIT);

        VK_CHECK(vkCreateImageView(_logical_device, &rview_info, nullptr, &drawImage->imageView));

        //add to deletion queues
        /*_mainDeletionQueue.push_function([=]() {
            vkDestroyImageView(_device, _drawImage.imageView, nullptr);
            vmaDestroyImage(_allocator, _drawImage.image, _drawImage.allocation);
            });*/

        _depth_image.imageFormat = VK_FORMAT_D32_SFLOAT;
        _depth_image.imageExtent = drawImageExtent;
        VkImageUsageFlags depthImageUsages{};
        depthImageUsages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        VkImageCreateInfo dimg_info = vkinit::image_create_info(_depth_image.imageFormat, depthImageUsages, drawImageExtent);

        //allocate and create the image
        vmaCreateImage(_vma_allocator, &dimg_info, &rimg_allocinfo, &_depth_image.image, &_depth_image.allocation, nullptr);

        //build a image-view for the draw image to use for rendering
        VkImageViewCreateInfo dview_info = vkinit::imageview_create_info(_depth_image.imageFormat, _depth_image.image, VK_IMAGE_ASPECT_DEPTH_BIT);

        VK_CHECK(vkCreateImageView(_logical_device, &dview_info, nullptr, &_depth_image.imageView));
    }

    void VKEngine::init_queue_families(vkb::Device vkbDevice) {
        _graphics_queue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
        _queue_families.graphicsFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
        _present_queue = vkbDevice.get_queue(vkb::QueueType::present).value();
        _queue_families.presentFamily = vkbDevice.get_queue_index(vkb::QueueType::present).value();
        _transfer_queue = vkbDevice.get_queue(vkb::QueueType::transfer).value();
        _queue_families.transferFamily = vkbDevice.get_queue_index(vkb::QueueType::transfer).value();
    }

    void VKEngine::init_vma() {
        VmaAllocatorCreateInfo allocatorInfo = vkinit::vma_allocator_create_info(GetVulkanApiVersion(), _instance, _logical_device, _physical_device);
        vmaCreateAllocator(&allocatorInfo, &_vma_allocator);
    }

    void VKEngine::init_commands() {
        //create a command pool for commands submitted to the graphics queue.
        //we also want the pool to allow for resetting of individual command buffers
        VkCommandPoolCreateInfo commandPoolInfo = vkinit::command_pool_create_info(_queue_families.graphicsFamily.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VK_CHECK(vkCreateCommandPool(_logical_device, &commandPoolInfo, nullptr, &_frames[i].commandPool));

            VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(_frames[i].commandPool, 1);

            VK_CHECK(vkAllocateCommandBuffers(_logical_device, &cmdAllocInfo, &_frames[i].commandBuffer));

            _frames[i].commandsInitialized = true;
        }

        //init imgui commands
        VK_CHECK(vkCreateCommandPool(_logical_device, &commandPoolInfo, nullptr, &_imgui_sync_data.commandPool));
        VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(_imgui_sync_data.commandPool, 1);
        VK_CHECK(vkAllocateCommandBuffers(_logical_device, &cmdAllocInfo, &_imgui_sync_data.commandBuffer));
    }

    void VKEngine::init_sync_structs() {
        // create syncronization structures
        //one fence to control when the gpu has finished rendering the frame,
        //and 2 semaphores to syncronize rendering with swapchain
        //we want the fence to start signalled so we can wait on it on the first frame
        VkFenceCreateInfo fenceCreateInfo = vkinit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
        VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphore_create_info();

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VK_CHECK(vkCreateFence(_logical_device, &fenceCreateInfo, nullptr, &_frames[i].renderFence));

            VK_CHECK(vkCreateSemaphore(_logical_device, &semaphoreCreateInfo, nullptr, &_frames[i].acquireSwapchainSemaphore));
            //VK_CHECK(vkCreateSemaphore(_logical_device, &semaphoreCreateInfo, nullptr, &_frames[i].renderSemaphore));

            _frames[i].syncingInitialized = true;
        }
        VK_CHECK(vkCreateFence(_logical_device, &fenceCreateInfo, nullptr, &_imgui_sync_data.fence));
    }

    void VKEngine::init_descriptors() {

        //create a descriptor pool that will hold 10 sets with 1 image each
        std::vector<DescriptorAllocator::PoolSizeRatio> sizes =
        {
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }
        };

        _descriptor_allocator.init_pool(_logical_device, 10, sizes);

        //make the descriptor set layout for our compute draw
        {
            DescriptorLayoutBuilder builder;
            builder.add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
            _draw_image_data.drawImageDescriptorLayout = builder.build(_logical_device, VK_SHADER_STAGE_COMPUTE_BIT);
        }

        //allocate a descriptor set for our draw image
        _draw_image_data.drawImageDescriptors = _descriptor_allocator.allocate(_logical_device, _draw_image_data.drawImageDescriptorLayout);

        VkDescriptorImageInfo imgInfo{};
        imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        imgInfo.imageView = _draw_image_data.allocatedImage.imageView;

        VkWriteDescriptorSet drawImageWrite = {};
        drawImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        drawImageWrite.pNext = nullptr;

        drawImageWrite.dstBinding = 0;
        drawImageWrite.dstSet = _draw_image_data.drawImageDescriptors;
        drawImageWrite.descriptorCount = 1;
        drawImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        drawImageWrite.pImageInfo = &imgInfo;

        vkUpdateDescriptorSets(_logical_device, 1, &drawImageWrite, 0, nullptr);

        //make sure both the descriptor allocator and the new layout get cleaned up properly
        /*_mainDeletionQueue.push_function([&]() {
            globalDescriptorAllocator.destroy_pool(_device);
            vkDestroyDescriptorSetLayout(_device, _drawImageDescriptorLayout, nullptr);
            });*/
    }

    void VKEngine::init_imgui() {
        // 1: create descriptor pool for IMGUI
        //  the size of the pool is very oversize, but it's copied from imgui demo
        //  itself.
        VkDescriptorPoolSize pool_sizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000;
        pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;

        VK_CHECK(vkCreateDescriptorPool(_logical_device, &pool_info, nullptr, &_imgui_sync_data.descriptorPool));

        // 2: initialize imgui library

        // this initializes the core structures of imgui
        ImGui::CreateContext();

        // this initializes imgui for SDL
        ImGui_ImplSDL3_InitForVulkan(_window->getSDLWindow());

        // this initializes imgui for Vulkan
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = _instance;
        init_info.PhysicalDevice = _physical_device;
        init_info.Device = _logical_device;
        init_info.Queue = _graphics_queue;
        init_info.DescriptorPool = _imgui_sync_data.descriptorPool;
        init_info.MinImageCount = 3;
        init_info.ImageCount = 3;
        init_info.UseDynamicRendering = true;

        //dynamic rendering parameters for imgui to use
        init_info.PipelineRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
        init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
        init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &_swapchain_image_format;


        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        ImGui_ImplVulkan_Init(&init_info);

        ImGui_ImplVulkan_CreateFontsTexture();

        //// add the destroy the imgui created structures
        //_mainDeletionQueue.push_function([=]() {
        //    ImGui_ImplVulkan_Shutdown();
        //    vkDestroyDescriptorPool(_device, imguiPool, nullptr);
        //    });
    }

    void VKEngine::init_default_data() {
        std::array<Vertex, 4> rect_vertices;

        rect_vertices[0].position = { 0.5,-0.5, 0 };
        rect_vertices[1].position = { 0.5,0.5, 0 };
        rect_vertices[2].position = { -0.5,-0.5, 0 };
        rect_vertices[3].position = { -0.5,0.5, 0 };

        rect_vertices[0].color = { 0,0, 0,1 };
        rect_vertices[1].color = { 0.5,0.5,0.5 ,1 };
        rect_vertices[2].color = { 1,0, 0,1 };
        rect_vertices[3].color = { 0,1, 0,1 };

        std::array<uint32_t, 6> rect_indices;

        rect_indices[0] = 0;
        rect_indices[1] = 1;
        rect_indices[2] = 2;

        rect_indices[3] = 2;
        rect_indices[4] = 1;
        rect_indices[5] = 3;

        rectangle = upload_mesh_buffer(rect_indices, rect_vertices);
        
        testMeshes = loadGltfMeshes(this, "models/basicmesh.glb").value();
        ////delete the rectangle data on engine shutdown
        //_mainDeletionQueue.push_function([&]() {
        //    destroy_buffer(rectangle.indexBuffer);
        //    destroy_buffer(rectangle.vertexBuffer);
        //    });

    }
#pragma endregion

#pragma region SwapChain
    void VKEngine::create_swapchain(uint32_t width, uint32_t height) {

        vkb::SwapchainBuilder swapchainBuilder{ _physical_device, _logical_device, _surface };

        //VK_FORMAT_R16G16B16A16_SFLOAT
        _swapchain_image_format = VK_FORMAT_B8G8R8A8_UNORM;

        vkb::Swapchain vkbSwapchain = swapchainBuilder
            //.use_default_format_selection()
            .set_desired_format(VkSurfaceFormatKHR{ .format = _swapchain_image_format, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
            //use vsync present mode
            .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
            .set_desired_extent(width, height)
            .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
            .build()
            .value();

        _swapchain_extent = vkbSwapchain.extent;

        _swapchain = vkbSwapchain.swapchain;

        int imageCount = vkbSwapchain.image_count;
        _swapchain_data.resize(imageCount);
        std::vector<VkImage> swapChainImages = vkbSwapchain.get_images().value();
        std::vector<VkImageView> swapchainImageViews = vkbSwapchain.get_image_views().value();

        VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphore_create_info();
      
        for (int i = 0; i < imageCount; i++) {
            SwapChainData swapChainData{};
            swapChainData.image = swapChainImages[i];
            swapChainData.imageView = swapchainImageViews[i];
            VK_CHECK(vkCreateSemaphore(_logical_device, &semaphoreCreateInfo, nullptr, &swapChainData.submitSemaphore));
            _swapchain_data[i] = swapChainData;
        }
    }

    void VKEngine::destroy_swapchain() {
        vkDestroySwapchainKHR(_logical_device, _swapchain, nullptr);
        for (auto swapChainData : _swapchain_data) {
            swapChainData.destroy_swapchain_data(_logical_device);
        }
    }
#pragma endregion

#pragma region Command Utils
    void VKEngine::immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function)
    {
        VK_CHECK(vkResetFences(_logical_device, 1, &_imgui_sync_data.fence));
        VK_CHECK(vkResetCommandBuffer(_imgui_sync_data.commandBuffer, 0));

        VkCommandBuffer cmd = _imgui_sync_data.commandBuffer;

        VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

        function(cmd);

        VK_CHECK(vkEndCommandBuffer(cmd));

        VkCommandBufferSubmitInfo cmdinfo = vkinit::command_buffer_submit_info(cmd);
        VkSubmitInfo2 submit = vkinit::submit_info(&cmdinfo, nullptr, nullptr);

        // submit command buffer to the queue and execute it.
        //  _renderFence will now block until the graphic commands finish execution
        VK_CHECK(vkQueueSubmit2(_graphics_queue, 1, &submit, _imgui_sync_data.fence));

        VK_CHECK(vkWaitForFences(_logical_device, 1, &_imgui_sync_data.fence, true, 9999999999));
    }
#pragma endregion

#pragma region Rendering
    void VKEngine::draw_imgui(VkCommandBuffer cmd, VkImageView targetImageView) {
        VkRenderingAttachmentInfo colorAttachment = vkinit::attachment_info(targetImageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        VkRenderingInfo renderInfo = vkinit::rendering_info(_draw_extent, &colorAttachment, nullptr);

        vkCmdBeginRendering(cmd, &renderInfo);

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

        vkCmdEndRendering(cmd);
    }

    VkShaderModule VKEngine::createShaderModule(const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(_logical_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

    void VKEngine::record_command_buffer(VkCommandBuffer cmd, uint32_t imageIndex, std::vector<RenderObject>* renderObjects) {
        AllocatedImage drawImage = _draw_image_data.allocatedImage;
        _draw_extent.width =  drawImage.imageExtent.width;
        _draw_extent.height = drawImage.imageExtent.height;

        VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

        vkutil::transition_image(cmd, drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

        draw_background(cmd);

        vkutil::transition_image(cmd, drawImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        vkutil::transition_image(cmd, _depth_image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

        draw_geometry(cmd);

        //transition the draw image and the swapchain image into their correct transfer layouts
        vkutil::transition_image(cmd, drawImage.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        vkutil::transition_image(cmd, _swapchain_data[imageIndex].image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        // execute a copy from the draw image into the swapchain
        vkutil::copy_image_to_image(cmd, drawImage.image, _swapchain_data[imageIndex].image, _draw_extent, _swapchain_extent);
        
        vkutil::transition_image(cmd, _swapchain_data[imageIndex].image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        draw_imgui(cmd, _swapchain_data[imageIndex].imageView);

        // set swapchain image layout to Present so we can draw it
        vkutil::transition_image(cmd, _swapchain_data[imageIndex].image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        VK_CHECK(vkEndCommandBuffer(cmd));
    }

    void VKEngine::draw_background(VkCommandBuffer cmd)
    {
        ComputeEffect& effect = *get_current_background_fx();
        //clear image
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, effect.pipeline);

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _background_pipeline_layout, 0, 1, &_draw_image_data.drawImageDescriptors, 0, nullptr);

        vkCmdPushConstants(cmd, _background_pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ComputePushConstants), &effect.data);

        vkCmdDispatch(cmd, std::ceil(_draw_extent.width / 16.0), std::ceil(_draw_extent.height / 16.0), 1);
    }

    void VKEngine::draw_geometry(VkCommandBuffer cmd)
    {
        //begin a render pass  connected to our draw image
        VkRenderingAttachmentInfo colorAttachment = vkinit::attachment_info(_draw_image_data.allocatedImage.imageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        VkRenderingAttachmentInfo depthAttachment = vkinit::depth_attachment_info(_depth_image.imageView, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

        VkRenderingInfo renderInfo = vkinit::rendering_info(_draw_extent, &colorAttachment, &depthAttachment);
        vkCmdBeginRendering(cmd, &renderInfo);

        //vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _triangle_pipeline);

        //set dynamic viewport and scissor
        VkViewport viewport = {};
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = _draw_extent.width;
        viewport.height = _draw_extent.height;
        viewport.minDepth = 0.f;
        viewport.maxDepth = 1.f;

        vkCmdSetViewport(cmd, 0, 1, &viewport);

        VkRect2D scissor = {};
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        scissor.extent.width = _draw_extent.width;
        scissor.extent.height = _draw_extent.height;

        vkCmdSetScissor(cmd, 0, 1, &scissor);

        //launch a draw command to draw 3 vertices
        //vkCmdDraw(cmd, 3, 1, 0, 0);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _mesh_pipeline);

        GPUDrawPushConstants push_constants;
        push_constants.worldMatrix = glm::mat4{ 1.f };
        push_constants.vertexBuffer = rectangle.vertexBufferAddress;

        vkCmdPushConstants(cmd, _mesh_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &push_constants);
        vkCmdBindIndexBuffer(cmd, rectangle.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(cmd, 6, 1, 0, 0, 0);

        glm::mat4 view = glm::translate(glm::mat4(1.0), glm::vec3{ 0,0,-5 });
        // camera projection
        glm::mat4 projection = glm::perspective(glm::radians(70.f), (float)_draw_extent.width / (float)_draw_extent.height, 10000.f, 0.1f);

        // invert the Y direction on projection matrix so that we are more similar
        // to opengl and gltf axis
        projection[1][1] *= -1;

        push_constants.worldMatrix = projection * view;
        push_constants.vertexBuffer = testMeshes[2]->gpuMeshBuffers.vertexBufferAddress;

        vkCmdPushConstants(cmd, _mesh_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &push_constants);
        vkCmdBindIndexBuffer(cmd, testMeshes[2]->gpuMeshBuffers.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(cmd, testMeshes[2]->surfaces[0].count, 1, testMeshes[2]->surfaces[0].startIndex, 0, 0);

        vkCmdEndRendering(cmd);
    }

#pragma endregion

#pragma region Buffers
    AllocatedBuffer VKEngine::create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) {
        // allocate buffer
        VkBufferCreateInfo bufferInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufferInfo.pNext = nullptr;
        bufferInfo.size = allocSize;

        bufferInfo.usage = usage;

        VmaAllocationCreateInfo vmaallocInfo = {};
        vmaallocInfo.usage = memoryUsage;
        vmaallocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
        AllocatedBuffer newBuffer;

        // allocate the buffer
        VK_CHECK(vmaCreateBuffer(_vma_allocator, &bufferInfo, &vmaallocInfo, &newBuffer.buffer, &newBuffer.allocation,
            &newBuffer.info));

        return newBuffer;
    }

    GPUMeshBuffers VKEngine::upload_mesh_buffer(std::span<uint32_t> indices, std::span<Vertex> vertices)
    {
        const size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
        const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

        GPUMeshBuffers newSurface;

        //create vertex buffer
        newSurface.vertexBuffer = create_buffer(vertexBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            VMA_MEMORY_USAGE_GPU_ONLY);

        //find the adress of the vertex buffer
        VkBufferDeviceAddressInfo deviceAdressInfo{ .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,.buffer = newSurface.vertexBuffer.buffer };
        newSurface.vertexBufferAddress = vkGetBufferDeviceAddress(_logical_device, &deviceAdressInfo);

        //create index buffer
        newSurface.indexBuffer = create_buffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VMA_MEMORY_USAGE_GPU_ONLY);


        AllocatedBuffer staging = create_buffer(vertexBufferSize + indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

        void* data;
        vmaMapMemory(_vma_allocator, staging.allocation, &data);

        // copy vertex buffer
        memcpy(data, vertices.data(), vertexBufferSize);
        // copy index buffer
        memcpy((char*)data + vertexBufferSize, indices.data(), indexBufferSize);

        immediate_submit([&](VkCommandBuffer cmd) {
            VkBufferCopy vertexCopy{ 0 };
            vertexCopy.dstOffset = 0;
            vertexCopy.srcOffset = 0;
            vertexCopy.size = vertexBufferSize;

            vkCmdCopyBuffer(cmd, staging.buffer, newSurface.vertexBuffer.buffer, 1, &vertexCopy);

            VkBufferCopy indexCopy{ 0 };
            indexCopy.dstOffset = 0;
            indexCopy.srcOffset = vertexBufferSize;
            indexCopy.size = indexBufferSize;

            vkCmdCopyBuffer(cmd, staging.buffer, newSurface.indexBuffer.buffer, 1, &indexCopy);
            });

        vmaUnmapMemory(_vma_allocator, staging.allocation);
        staging.destroy_allocated_buffer(_vma_allocator);

        return newSurface;

    }
#pragma endregion

#pragma region Getters

#pragma endregion
}
}
}