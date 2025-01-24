#define GLFW_INCLUDE_VULKAN
#define NOMINMAX

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <stdlib.h>     /* srand, rand */
#include <fstream>

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>
#include <chrono>

#include "vertexData.h"
#include "RenderObject.h"
#include "TransformObject.h"
#include "Transform.h"
#include "Camera.h"
#include "VKEngine.h"

class HelloTriangleApplication {
public:
    void run() {
        initWindow();
        renderer.InitVulkan(window);
        createObjects();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow* window;
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;
    PenguinEngine::Graphics::VKEngine renderer;

    std::vector<RenderObject> _renderedObjects;

    const uint16_t SPAWN_COUNT = 20;
    const float SPAWN_SIZE = 10.0f;

    Camera camera;
    RenderObject squareObject;

    void initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
        app->renderer.SetFrameBufferResized(true);
    }



    void createObjects() {
        camera = Camera(75.0f, renderer.GetSwapChainAspectRatio(), 0.1f, 200.0f);
        camera.transform.LookAt(glm::vec3(0.0f, 3.0f, 12.0f), glm::vec3(0, 0, 0), glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)));

        _renderedObjects.resize(SPAWN_COUNT);
        for (int i = 0; i < _renderedObjects.size(); i++) {
            float xPos = (rand() % 100) / 100.0f, yPos = (rand() % 100) / 100.0f, zPos = (rand() % 100) / 100.0f;
            xPos = xPos - 0.5f;
            yPos = yPos - 0.5f;
            RenderObject renderObj = RenderObject();
            renderObj.transform.SetPosition(glm::vec3(xPos * SPAWN_SIZE, yPos * 0.2f * SPAWN_SIZE, zPos * SPAWN_SIZE));
            renderObj.rotOffset = (rand() % 100) / 100.0f * 10.f;
            _renderedObjects[i] = renderObj;
        }
    }

    void updateObjects() {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        int val = (int)(glm::round(time) / 0.3f);
 
        for (int i = 0; i < _renderedObjects.size(); i++) {
            _renderedObjects[i].transform.SetRotation_Matrix(glm::rotate(glm::mat4(1.0), (time + _renderedObjects[i].rotOffset) * glm::radians(40.0f), glm::vec3(0, 1.0, 0)));
            //_renderedObjects[i].transform.setLocalToWorldMatrix(glm::mat4(i+1));
        }

        camera.aspectRatio = renderer.GetSwapChainAspectRatio();
        //camera.transform.LookAt(glm::vec3(0.0, 2.0f, 8.0f), glm::vec3(0, 0, 0), glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)));
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            updateObjects();
            renderer.DrawFrame(window, camera, &_renderedObjects);
        }
        renderer.WaitRendererIdle();
    }

    void cleanup() {
        renderer.Cleanup();
        glfwDestroyWindow(window);
        glfwTerminate();
    }
};


int main(int argc, char* argv[]) {
    unsigned seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::srand(seed);

    HelloTriangleApplication app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}