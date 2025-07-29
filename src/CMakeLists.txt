file(GLOB MAIN_SRC 
    "main.cpp"
)

set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)

add_executable(${PROJECT_NAME} ${MAIN_SRC})

target_compile_definitions(${PROJECT_NAME} PUBLIC RESOUCES_DIR="${CMAKE_BINARY_DIR}/resources")

#target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/src")


target_link_libraries(${PROJECT_NAME}
    PRIVATE
    pengine
)

find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${SHADER_SOURCE_DIR}/*.frag"
    "${SHADER_SOURCE_DIR}/*.vert"
    "${SHADER_SOURCE_DIR}/*.comp"
    )

foreach(GLSL_SOURCE ${GLSL_SOURCE_FILES})
  message(STATUS "BUILDING SHADER ${GLSL_SOURCE}")
  get_filename_component(FILE_NAME ${GLSL_SOURCE} NAME)
  MESSAGE(STATUS ${FILE_NAME})
  set(SPIRV_OUT "${SHADER_BINARY_DIR}/${FILE_NAME}.spv")
  add_custom_command(POST_BUILD
    OUTPUT ${SPIRV_OUT}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_BINARY_DIR}
    COMMAND ${GLSL_VALIDATOR} -o ${SPIRV_OUT} ${GLSL_SOURCE} 
    DEPENDS ${GLSL_SOURCE}
    COMMENT "Outputting ${SPIRV_OUT}"
  )
  list(APPEND SPIRV_BINARY_FILES ${SPIRV_OUT})
endforeach()


add_custom_target(
    Shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
    )

add_dependencies(${PROJECT_NAME} Shaders)

#add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_BINARY_DIR}
#COMMAND ${CMAKE_COMMAND} -E copy_directory
#    ${SHADER_SOURCE_DIR}
#    ${SHADER_BINARY_DIR}
#    )
