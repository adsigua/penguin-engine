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
    fastgltf
    GIT_REPOSITORY https://github.com/spnda/fastgltf.git
    GIT_TAG v0.9.0
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
    vk_bootstrap
    GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap
    GIT_TAG v1.4.310 #suggest using a tag so the library doesn't update whenever new commits are pushed to a branch
)

FetchContent_GetProperties(imgui)
if (NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif ()

FetchContent_MakeAvailable(
    glfw
    glm
    tinygltf
    fastgltf
    SDL3
    imgui
    vk_bootstrap
)

set(IMGUI_INCLUDE_DIR ${imgui_SOURCE_DIR}/)
set(IMGUI_INCLUDE_BACKENDS_DIR ${imgui_SOURCE_DIR}/backends/)
file(GLOB IMGUI_SOURCES ${imgui_SOURCE_DIR}/*.cpp ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp)
file(GLOB IMGUI_HEADERS ${imgui_SOURCE_DIR}/*.h ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.h ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.h)
add_library(imgui STATIC ${IMGUI_SOURCES} ${IMGUI_HEADERS})

target_link_libraries(imgui
    PUBLIC
    Vulkan::Vulkan
    SDL3::SDL3-static
)

target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
#target_link_directories(imgui PUBLIC ${IMGUI_INCLUDE_DIR} ${IMGUI_HEADERS})

# START ADDITION
set(SDL2IMAGE_INSTALL OFF)
set(BUILD_SHARED_LIBS FALSE)
# END ADDITION

project(penguin_engine LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(includes)
include_directories("${CMAKE_SOURCE_DIR}/src")

add_subdirectory(src/pengine)
add_subdirectory(src)


#if ("${GLSL_VALIDATOR}")
#    MESSAGE(STATUS "Found glslangValidator")
#else()
#    MESSAGE(STATUS "ERROR Did not find ${GLSL_VALIDATOR}")
#endif()

#add_custom_command(
#  COMMAND
#    ${CMAKE_COMMAND} -E make_directory ${SHADER_BINARY_DIR}
#  OUTPUT ${SHADER_BINARY_DIR}
#  COMMENT "Creating ${SHADER_BINARY_DIR}"
#)

#file(MAKE_DIRECTORY ${SHADER_BINARY_DIR})


