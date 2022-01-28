cmake_minimum_required(VERSION 3.14)

project(
    OpenGLStarter 
    VERSION 1.0
    LANGUAGES CXX
)

message(${CMAKE_SOURCE_DIR})

include(FetchContent)

# =============================
# Libraries
# =============================
FetchContent_Declare(
    glm
    URL "file://${CMAKE_SOURCE_DIR}/.zero-install/glm-0.9.9.8.zip"
)

FetchContent_Declare(
    glfw 
    URL "file://${CMAKE_SOURCE_DIR}/.zero-install/glfw-3.3.6.zip"
)
option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)

FetchContent_Declare(
    glad
    URL "file://${CMAKE_SOURCE_DIR}/.zero-install/glad-4.6-no-extensions.zip"
)

# =============================
# Downloading Content
# =============================
FetchContent_MakeAvailable(glm glad glfw)

# =============================
# Create Executable
# =============================
set(EXEC_NAME ogls)

add_executable(
    ${EXEC_NAME} 
    main.cpp
)

target_link_libraries(
    ${EXEC_NAME}
    glm
    glfw
    glad
)

set_property(TARGET ${EXEC_NAME} PROPERTY CXX_STANDARD 17)
