
#pragma once 

#include "vk_types.h"
#include "vk_enum_string_helper.h"

namespace penguin_engine {
namespace graphics {
namespace vulkan {
namespace vkutil {

#define VK_CHECK(x)                                                     \
    do {                                                                \
        VkResult err = x;                                               \
        if (err) {                                                      \
            printf("Detected Vulkan error: %s", string_VkResult(err));  \
            abort();                                                    \
        }                                                               \
    } while (0)

	void transition_image(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
	void copy_image_to_image(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize);

	bool load_shader_module(const char* filePath, VkDevice device, VkShaderModule* outShaderModule);
}
}
}
}
