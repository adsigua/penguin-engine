file(GLOB GRAPHICS_CORE_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/graphics/model_loader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphics/model_loader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/graphics/graphics_types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/graphics/graphics_includes.h
)

add_library(pengine_graphics STATIC ${GRAPHICS_CORE_SRC})

target_include_directories(pengine_graphics PUBLIC "${CMAKE_SOURCE_DIR}/src" "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_directories(pengine_graphics PUBLIC "${CMAKE_SOURCE_DIR}/src" "${CMAKE_CURRENT_SOURCE_DIR}")

target_compile_definitions(pengine_graphics PUBLIC SHADER_DIR="${CMAKE_BINARY_DIR}/src/shaders")

target_link_libraries(pengine_graphics
    PRIVATE
    pengine_core
    tinygltf
    fastgltf
)
