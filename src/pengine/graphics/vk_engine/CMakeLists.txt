
file(GLOB VK_ENGINE_SRC
    vk_types.h
    vk_engine.h
    vk_engine.cpp
    vk_vma_Usage.h
    vk_vma_Usage.cpp
    vk_initializers.h
    vk_initializers.cpp
    vk_vertex_data.h
)

add_library(vk_engine STATIC ${VK_ENGINE_SRC})

target_include_directories(vk_engine PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_directories(vk_engine PRIVATE "${CMAKE_SOURCE_DIR}/src")

target_link_libraries(vk_engine
    PRIVATE
    pengine_graphics
    pengine_core
    tinygltf
    PUBLIC
    vk-bootstrap::vk-bootstrap
    Vulkan::Vulkan
)

