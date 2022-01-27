#include <chrono>
#include <cstdlib>
#include <glad/gl.h>
#include <iostream>
#include <thread>
#include <vector>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
#include <glm/ext/matrix_transform.hpp>  // glm::translate, glm::rotate, glm::scale
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

const char* vertShader = R"(
#version 460

// Recieve this information from the CPU
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

uniform mat4 mvp;

// Passing this to the fragment shader
layout(location = 0) out vec3 vColor;

void main()
{
    vColor = inColor;
    gl_Position = mvp * vec4(inPosition, 1.0);
}
)";

const char* fragShader = R"(
#version 460

precision mediump float;
precision highp int;

// Recieve this from vertex shader (interpolated)
layout(location = 0) in vec3 vColor;

layout(location = 0) out highp vec4 outFragColor;

void main()
{
    outFragColor = vec4(vColor, 1.0);
}
)";

static void error_callback(int error, const char* description)
{
    std::cerr << "Error: " << description << std::endl;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

const std::vector<float> positions = {
    1.0f, -1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f,
    0.0f, 1.0f, 0.0f};

const std::vector<float> colors = {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f};

const auto WIDTH = 640;
const auto HEIGHT = 480;

int main(void)
{
    // =================================================================
    // Window & OpenGL setup
    // =================================================================
    GLFWwindow* window;
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    if (!gladLoadGL(glfwGetProcAddress))
    {
        std::cout << "Failed to load OpenGL.";
        exit(EXIT_FAILURE);
    }

    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        std::cout << "OpenGL started with error code: " << err;
        exit(EXIT_FAILURE);
    }

    int majorVersion, minorVersion;
    glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
    std::cout << "OpenGL Version: " << majorVersion << "." << minorVersion
              << std::endl;

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glViewport(0, 0, WIDTH, HEIGHT);
    glEnable(GL_DEPTH_TEST);

    // No error checks! Lots of them are there

    // =================================================================
    // Create Shaders
    // =================================================================
    auto vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertShader, NULL);
    glCompileShader(vertex_shader);

    auto fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragShader, NULL);
    glCompileShader(fragment_shader);

    // =================================================================
    // Create Program (Consists of two shaders)
    // =================================================================
    auto program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glUseProgram(program);

    // =================================================================
    // Create VAO (Consists of one or more VBO)
    // =================================================================
    GLuint vao{};
    glGenVertexArrays(1, &vao);

    // =================================================================
    // Create VBO
    // =================================================================
    GLuint positionVBO{};
    glGenBuffers(1, &positionVBO);
    glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(positions[0]), positions.data(), GL_STATIC_DRAW);

    GLuint colorVBO{};
    glGenBuffers(1, &colorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(colors[0]), colors.data(), GL_STATIC_DRAW);

    // =================================================================
    // Bind VBOs to VAO, and buffers to locations
    // =================================================================
    glBindVertexArray(vao);

    // hard code location
    glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Ask for location
    auto colorLocation = glGetAttribLocation(program, "inColor");
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glEnableVertexAttribArray(colorLocation);
    glVertexAttribPointer(colorLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // =================================================================
    // Setup for draw call
    // =================================================================
    auto mvpLocation = glGetUniformLocation(program, "mvp");
    std::chrono::time_point<std::chrono::steady_clock> startTime, endTime;

    // =================================================================
    // Setup transformation matrices
    // =================================================================
    glm::mat4 projection = glm::perspective(glm::radians(70.0f), (float)WIDTH / (float)HEIGHT, 0.01f, 1000.0f);

    auto cameraPos = glm::vec3(0, 0, 2);
    auto up = glm::vec3(0, 1, 0);
    auto view = glm::lookAt(cameraPos, glm::vec3(0, 0, 0), up);

    auto model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));

    // =================================================================
    // Render loop
    // =================================================================
    while (!glfwWindowShouldClose(window))
    {
        endTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::milli>(endTime - startTime).count();
        startTime = std::chrono::high_resolution_clock::now();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        model = glm::rotate(model, glm::radians(0.05f * time), glm::vec3(0.0f, 1.0f, 0.0f));
        auto mvp = projection * view * model;

        glUseProgram(program);
        glBindVertexArray(vao);
        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();

        // ~60 fps
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(16ms);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    exit(EXIT_SUCCESS);
}
