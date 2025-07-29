#pragma once

#include "vk_engine.h"
#include "vk_types.h"

#include <filesystem>

namespace penguin_engine {
namespace graphics {
namespace vulkan {
    class VKEngine;

    std::optional<std::vector<std::shared_ptr<GPUMeshAsset>>> loadGltfMeshes(VKEngine* engine, std::filesystem::path filePath);
}
}
}
