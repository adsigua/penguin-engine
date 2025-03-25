file(GLOB PENGINE_CORE_SRC
    camera.h
    camera.cpp
    random.h
    random.cpp
    transform.h
    transform.cpp
    time.h
    time.cpp
    window.h
    window.cpp
    transform_object.h
    render_object.h
    constants.h
    utilities.h
    core_includes.h
)

add_library(pengine_core STATIC ${PENGINE_CORE_SRC})

target_compile_definitions(pengine_core
    PUBLIC 
    RESOURCES_PATH="${CMAKE_SOURCE_DIR}/resources/"
    SOURCE_PATH="${CMAKE_SOURCE_DIR}/src/")

target_link_libraries(pengine_core
    PUBLIC
    glm
    glfw
    imgui
    SDL3::SDL3-static
)

