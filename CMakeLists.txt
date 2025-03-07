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

#FetchContent_Declare(vma
#    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
#    GIT_TAG master
#    GIT_PROGRESS ON)

FetchContent_MakeAvailable(glfw)
FetchContent_MakeAvailable(glm)
FetchContent_MakeAvailable(tinygltf)
#FetchContent_MakeAvailable(vma)

project(penguin-engine LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*")
add_executable(penguin-engine "${MY_SOURCES}")

#target_compile_definitions(penguin-engine 
#    PUBLIC 
#    RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/"
#    SOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/src/")

target_compile_definitions(penguin-engine 
    PUBLIC 
    RESOURCES_PATH="../../resources/"
    SOURCE_PATH="../../src/")

target_link_libraries(penguin-engine
    PRIVATE
	glfw
	glm::glm
    tinygltf
    Vulkan::Vulkan
)
