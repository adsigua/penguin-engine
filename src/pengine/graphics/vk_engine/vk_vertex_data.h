#pragma once
#ifndef PENGUIN_VK_VERTEX_DATA
#define PENGUIN_VK_VERTEX_DATA

#include <array>
#include "vk_types.h"

namespace PenguinEngine {
namespace Graphics {
namespace Vulkan {

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }

    bool operator==(const Vertex& other) const {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }
};
}
}
}

//template<> struct std::hash<PenguinEngine::Graphics::VK_Renderer::Vertex> {
//    size_t operator()(PenguinEngine::Graphics::VK_Renderer::Vertex const& vertex) const {
//        return ((std::hash<glm::vec3>()(vertex.pos) ^
//            (std::hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
//            (std::hash<glm::vec2>()(vertex.texCoord) << 1);
//    }
//};

#endif