#pragma once
#ifndef PENGUIN_GRAPHICS_MODEL_LOADER
#define PENGUIN_GRAPHICS_MODEL_LOADER

#include <filesystem>
#include <graphics/graphics_types.h>

namespace penguin_engine {
namespace graphics {

    std::optional<std::vector<std::shared_ptr<MeshAsset>>> loadGltfMeshes(std::filesystem::path filePath);
}
}
#endif