file(GLOB MAIN_SRC 
    "main.cpp"
)

add_executable(${PROJECT_NAME} ${MAIN_SRC})

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src")

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    pengine
)
