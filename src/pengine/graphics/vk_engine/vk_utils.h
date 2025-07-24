
#pragma once 
#ifndef PENGUIN_VK_UTILS
#define PENGUIN_VK_UTILS

#include <vk_types.h>

namespace penguin_engine {
namespace graphics {
namespace vulkan {
namespace vkutil {

	void transition_image(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);

}
}
}
}
#endif