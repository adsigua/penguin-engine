cmake_minimum_required(VERSION 3.10)

include(FetchContent)

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

FetchContent_MakeAvailable(glfw)
FetchContent_MakeAvailable(glm)

project(penguin-engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
add_executable(penguin-engine "src/main.cpp")

target_link_libraries(penguin-engine
	PRIVATE
	glfw
	glm
)
