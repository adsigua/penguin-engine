add_library(pengine INTERFACE "")

add_subdirectory(core)
add_subdirectory(graphics)
add_subdirectory(graphics/vk_engine)

target_link_libraries(pengine
    INTERFACE
    pengine_graphics
    pengine_core
    vk_engine
)