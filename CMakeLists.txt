cmake_minimum_required(VERSION 3.2)

include(FetchContent)

include_directories("${CMAKE_SOURCE_DIR}/includes/")
find_package(Vulkan REQUIRED FATAL_ERROR)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW ON
    GIT_PROGRESS ON
    FIND_PACKAGE_ARGS 3.4)

FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW ON
    GIT_PROGRESS ON)

FetchContent_Declare(tinygltf
    GIT_REPOSITORY https://github.com/syoyo/tinygltf.git
    GIT_TAG release
    GIT_PROGRESS ON)

FetchContent_Declare(SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG f6864924f76e1a0b4abaefc76ae2ed22b1a8916e
    GIT_SHALLOW ON
    GIT_PROGRESS ON)

# START ADDITION
set(SDL2IMAGE_INSTALL OFF)
set(BUILD_SHARED_LIBS FALSE)
# END ADDITION

FetchContent_MakeAvailable(glfw)
FetchContent_MakeAvailable(glm)
FetchContent_MakeAvailable(tinygltf)
FetchContent_MakeAvailable(SDL3)

project(penguin-engine LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE TOP_SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*")
file(GLOB_RECURSE VULKAN_SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/*")
set(MAIN_SRC ${TOP_SRC} ${VULKAN_SRC})
add_executable(penguin-engine "${MAIN_SRC}")

target_compile_definitions(penguin-engine 
    PUBLIC 
    RESOURCES_PATH="../../resources/"
    SOURCE_PATH="../../src/")

target_link_libraries(penguin-engine
    PRIVATE
	glfw
	glm::glm
    tinygltf
    SDL3::SDL3-static
    Vulkan::Vulkan
)
