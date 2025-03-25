cmake_minimum_required(VERSION 3.5)

include(FetchContent)

find_package(Vulkan REQUIRED)
if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan SDK not found. Please install Vulkan SDK and ensure it's in your CMake search path.")
else()
    message("Vulkan found!")
endif()
#get_filename_component(VULKAN_SDK_ROOT ${Vulkan_LIBRARY} DIRECTORY)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW ON
    GIT_PROGRESS ON
    FIND_PACKAGE_ARGS 3.4
)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW ON
    GIT_PROGRESS ON
)

FetchContent_Declare(
    tinygltf
    GIT_REPOSITORY https://github.com/syoyo/tinygltf.git
    GIT_TAG release
    GIT_PROGRESS ON
)

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG f6864924f76e1a0b4abaefc76ae2ed22b1a8916e
    GIT_SHALLOW ON
    GIT_PROGRESS ON
)
    
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG 4806a1924ff6181180bf5e4b8b79ab4394118875 #suggest using a tag so the library doesn't update whenever new commits are pushed to a branch
    GIT_SHALLOW ON
    GIT_PROGRESS ON
)
    
FetchContent_Declare(
    fetch_vk_bootstrap
    GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap
    GIT_TAG v1.4.310 #suggest using a tag so the library doesn't update whenever new commits are pushed to a branch
)

FetchContent_MakeAvailable(
    glfw
    glm
    tinygltf
    SDL3
    imgui
    fetch_vk_bootstrap
)

FetchContent_GetProperties(imgui)
if (NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif ()
set(IMGUI_INCLUDE_DIR ${imgui_SOURCE_DIR}/)
file(GLOB IMGUI_SOURCES ${imgui_SOURCE_DIR}/*.cpp)
file(GLOB IMGUI_HEADERS ${imgui_SOURCE_DIR}/*.h)
add_library(imgui STATIC ${IMGUI_SOURCES} ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp)
target_link_libraries(imgui
    PUBLIC
    Vulkan::Vulkan
    SDL3::SDL3-static
)
target_include_directories(imgui PUBLIC ${IMGUI_INCLUDE_DIR})

# START ADDITION
set(SDL2IMAGE_INSTALL OFF)
set(BUILD_SHARED_LIBS FALSE)
# END ADDITION

project(penguin_engine LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(includes)

#link_directories(${VULKAN_SDK_ROOT})
#include_directories(src/pengine)
#include_directories(src/vulkan)

add_subdirectory(src/pengine)
add_subdirectory(src)
#add_subdirectory(src/pengine/graphics/vk_engine)

#target_link_libraries(src PUBLIC
#)



