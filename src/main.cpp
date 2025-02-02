#include <stdlib.h>     /* srand, rand */

#include <vector>
#include <iostream>

#include "TransformObject.h"
#include "Transform.h"
#include "VKEngine.h"
#include "Time.h"

class HelloTriangleApplication {
public:
    /*HelloTriangleApplication() {
        
    }*/

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

    const uint16_t SPAWN_COUNT = 2;
    const float SPAWN_SIZE = 2.0f;

    Camera camera;
    RenderObject squareObject;
    //std::vector<PenguinEngine::Graphics::SwapChainData> _swapChainData;

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
        camera.transform.LookAt(glm::vec3(0.0f, 1.0f, 5.0f), glm::vec3(0, 0, 0), glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)));

        _renderedObjects.resize(SPAWN_COUNT);
        for (int i = 0; i < _renderedObjects.size(); i++) {
            float xPos = (rand() % 100) / 100.0f, yPos = (rand() % 100) / 100.0f, zPos = (rand() % 100) / 100.0f;
            xPos = xPos - 0.5f;
            yPos = yPos - 0.5f;
            RenderObject renderObj = RenderObject();
            renderObj.transform.SetPosition(glm::vec3(xPos * SPAWN_SIZE, yPos * 0.5f * SPAWN_SIZE, zPos * SPAWN_SIZE));
            renderObj.rotOffset = (rand() % 100) / 100.0f * 10.f;
            _renderedObjects[i] = renderObj;
        }
        glm::mat4 objMat = glm::mat4(1.0);
        _renderedObjects[0].transform.SetRotation_Matrix(objMat);
    }


    void updateObjects() {
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        int val = (int)(glm::round(time) / 0.3f);
 
        for (int i = 0; i < _renderedObjects.size(); i++) {
            _renderedObjects[i].transform.Rotate(glm::radians(20.0f) * PenguinEngine::Time::getDeltaTime(), glm::vec3(0.0, 1.0, 0.0));
        }

        camera.aspectRatio = renderer.GetSwapChainAspectRatio();
        //camera.transform.LookAt(glm::vec3(0.0, 2.0f, 8.0f), glm::vec3(0, 0, 0), glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)));
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            PenguinEngine::Time::Tick();
            glfwPollEvents();
            updateObjects();
            renderer.DrawFrame(camera, &_renderedObjects);
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
    PenguinEngine::Time::Init();

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