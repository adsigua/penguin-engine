
file(GLOB VK_ENGINE_SRC
    vk_vma_Usage.cpp
    vk_initializers.cpp
    vk_descriptors.cpp
    vk_utils.cpp
    vk_loader.cpp
    vk_pipelines.cpp
    vk_engine.h
    vk_engine.cpp
    vk_vma_Usage.h
    vk_types.h
    vk_initializers.h
    vk_vertex_data.h
    vk_descriptors.h
    vk_utils.h
    vk_loader.h
    vk_pipelines.h
    vk_includes.h
)

add_library(vk_engine STATIC ${VK_ENGINE_SRC})

#target_include_directories(vk_engine PUBLIC "${CMAKE_SOURCE_DIR}/src" "${CMAKE_CURRENT_SOURCE_DIR}")
#target_link_directories(vk_engine PUBLIC "${CMAKE_SOURCE_DIR}/src" "${CMAKE_CURRENT_SOURCE_DIR}")

#target_include_directories(vk_engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
#target_link_directories(vk_engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_include_directories(vk_engine PUBLIC "${vk_bootstrap_SOURCE_DIR}/src")

target_link_libraries(vk_engine
    PRIVATE
    pengine_graphics
    pengine_core
    tinygltf
    fastgltf
    vk-bootstrap::vk-bootstrap
    Vulkan::Vulkan
)

