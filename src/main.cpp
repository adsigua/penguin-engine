//#include <vector>
//#include <unordered_map>
//#include <cmath>

#include <iostream>

#include <SDL3/SDL.h>
#include <pengine/core/core_includes.h>
#include <pengine/graphics/vk_engine/vk_engine.h>

#include <thread>

namespace PenguinEngine {


struct TimeDelayer {
    float nextTime;
    float requiredTime;
};

struct MouseInput {
    glm::vec2 startPosition;
    glm::vec2 deltaPosition;
    glm::vec2 position;
    int buttonState;
};

class PenguinEngineApplication {
public:
    

    void run() {
        initialize();
        mainLoop();
        cleanup();
    }

private:
    bool _isInitialized{ false };
    int _frameNumber{ 0 };
    bool stop_rendering{ false };

    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    Graphics::VKEngine _renderer;
    Window _window;

    std::vector<RenderObject> _renderedObjects;

    std::unordered_map<int, bool> _keyStates;
    std::unordered_map<int, MouseInput> _mouseStates;

    double _prevCursorX, _prevCursorY;
    glm::vec2 _prevCursorPos;
    glm::vec3 _prevCursorVPos;

    glm::vec2 windowSize;

    Camera _camera;

    TimeDelayer _rightClickChecker;

    void initialize() {
        _window.initialize();
        //_renderer.InitVulkan(window);
        createObjects();
    }

    /*void initInputStates() {
        glfwSetKeyCallback(window, inputCallBack);
        glfwSetMouseButtonCallback(window, mouseCallBack);

        _rightClickChecker.requiredTime = 0.01f;
        _rightClickChecker.nextTime = 0.0f;

        const int keys[] = {
            GLFW_KEY_R, GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_SPACE, GLFW_KEY_LEFT_CONTROL, GLFW_KEY_LEFT, GLFW_KEY_RIGHT, GLFW_KEY_UP, GLFW_KEY_DOWN
        };
        for (auto key : keys) {
            _keyStates[key] = false;
        }
        const int mouseInputs[] = {
            GLFW_MOUSE_BUTTON_LEFT, GLFW_MOUSE_BUTTON_RIGHT, GLFW_MOUSE_BUTTON_MIDDLE
        };
        for (auto input : mouseInputs) {
            _mouseStates[input] = MouseInput{};
            _mouseStates[input].buttonState = GLFW_RELEASE;
        }
        glfwGetCursorPos(window, &_prevCursorX, &_prevCursorY);
    }*/

    /*static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<PenguinEngineApplication*>(glfwGetWindowUserPointer(window));
        app->_renderer.SetFrameBufferResized(true);
        app->windowSize = glm::vec2(width, height);
    }

    static void inputCallBack(GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto app = reinterpret_cast<PenguinEngineApplication*>(glfwGetWindowUserPointer(window));
        app->handleInput(key, scancode, action, mods);
    }

    static void mouseCallBack(GLFWwindow* window, int button, int action, int mods) {
        auto app = reinterpret_cast<PenguinEngineApplication*>(glfwGetWindowUserPointer(window));
        app->handleMouseInput(button, action, mods);
    }*/

    void mainLoop() {
        SDL_Event e;
        bool bQuit = false;

        // main loop
        while (!bQuit) {
            // Handle events on queue
            while (SDL_PollEvent(&e) != 0) {
                // close the window when user alt-f4s or clicks the X button
                if (e.type == SDL_EVENT_QUIT) {
                    bQuit = true;
                } 
                
                if (e.type == SDL_EVENT_WINDOW_MINIMIZED) {
                    stop_rendering = true;
                }else if (e.type == SDL_EVENT_WINDOW_RESTORED) {
                    stop_rendering = false;
                }

                if (e.type == SDL_EVENT_WINDOW_RESIZED) {

                }
            }

            // do not draw if we are minimized
            if (stop_rendering) {
                // throttle the speed to avoid the endless spinning
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            draw();
        }
    }

    void createObjects() {
        _camera = Camera(75.0f, Window::getWindowAspectRatio(), 0.1f, 200.0f);
        _camera.transform.SetPosition(glm::vec3(0.0f, 0.0f, CAMERA_DISTANCE));
        _camera.transform.LookAt(glm::vec3(0, 0, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        _renderedObjects.resize(SPAWN_COUNT);
        for (int i = 0; i < _renderedObjects.size(); i++) {
            glm::vec3 randomPos = Random::getRandomInUnitSphere();
            randomPos.x -= -0.5f;
            randomPos.y -= - 0.5f;
            RenderObject renderObj = RenderObject();
            renderObj.transform.SetPosition(glm::vec3(randomPos.x * SPAWN_SIZE, randomPos.y * 0.5f * SPAWN_SIZE, -randomPos.z * SPAWN_SIZE));
            renderObj.transform.SetScale(glm::vec3(1.0f));
            renderObj.transform.SetRotation_Euler(glm::vec3(0.0f, Random::getRandomAngleRadians(), 0.0f));

            _renderedObjects[i] = renderObj;
        }
    }

    void updateObjects() {
        for (int i = 0; i < _renderedObjects.size(); i++) {
            _renderedObjects[i].transform.Rotate(glm::radians(5.0f) * Time::getDeltaTime(), glm::vec3(0.0, 1.0, 0.0));
        }

        _camera.aspectRatio = _renderer.GetSwapChainAspectRatio();
    }

    void checkForInput() {
        //glfwPollEvents();
        //checkKeyboardInput();
        //checkMouseInput();
    }

    void draw() {

    }

    //void checkKeyboardInput() {
    //    float speedMult = _keyStates[GLFW_KEY_LEFT_SHIFT] ? CAMERA_FAST_SPEED_MULTIPLIER : 1.0f;

    //    if (_keyStates[GLFW_KEY_W]) {
    //        _camera.moveCamera(_camera.transform.getForward(), speedMult);
    //    }
    //    if (_keyStates[GLFW_KEY_A]) {
    //        _camera.moveCamera(_camera.transform.getRight(), speedMult);
    //    }
    //    if (_keyStates[GLFW_KEY_S]) {
    //        _camera.moveCamera(-_camera.transform.getForward(), speedMult);
    //    }
    //    if (_keyStates[GLFW_KEY_D]) {
    //        _camera.moveCamera(-_camera.transform.getRight(), speedMult);
    //    }
    //    if (_keyStates[GLFW_KEY_SPACE]) {
    //        _camera.moveCamera(_camera.transform.getUp(), speedMult);
    //    }
    //    if (_keyStates[GLFW_KEY_LEFT_CONTROL]) {
    //        _camera.moveCamera(-_camera.transform.getUp(), speedMult);
    //    }

    //    glm::vec2 rotateDir = glm::vec2(0.0f, 0.0f);
    //    if (_keyStates[GLFW_KEY_LEFT]) {
    //        rotateDir.x = 1;
    //    }
    //    else if (_keyStates[GLFW_KEY_RIGHT]) {
    //        rotateDir.x = -1;
    //    }
    //    else if (_keyStates[GLFW_KEY_UP]) {
    //        rotateDir.y = -1;
    //    }
    //    else if (_keyStates[GLFW_KEY_DOWN]) {
    //        rotateDir.y = 1;
    //    }
    //    if (rotateDir.x != 0 || rotateDir.y != 0) {
    //        _camera.rotateCamera(rotateDir);
    //    }
    //}

    //void checkMouseInput() {
    //    //double xpos, ypos;
    //    //glfwGetCursorPos(window, &xpos, &ypos);

    //    //if (_mouseStates[GLFW_MOUSE_BUTTON_RIGHT].buttonState == GLFW_RELEASE && _mouseStates[GLFW_MOUSE_BUTTON_MIDDLE].buttonState == GLFW_RELEASE) {
    //    //    return;
    //    //}

    //    //if (_mouseStates[GLFW_MOUSE_BUTTON_RIGHT].buttonState == GLFW_PRESS) {
    //    //    _prevCursorX = xpos;
    //    //    _prevCursorY = ypos;
    //    //    _mouseStates[GLFW_MOUSE_BUTTON_RIGHT].buttonState = GLFW_REPEAT;
    //    //} else if (std::abs(xpos - _prevCursorX) >= CAMERA_TURN_MIN_SCREEN_DELTA || std::abs(ypos - _prevCursorY) >= CAMERA_TURN_MIN_SCREEN_DELTA) {
    //    //    double dX = xpos - _prevCursorX;
    //    //    double dY = ypos - _prevCursorY;

    //    //    float xNDC = dX / windowSize.x;
    //    //    float yNDC = dY / windowSize.y;

    //    //    glm::vec2 delta = glm::vec2(-xNDC, yNDC) / 0.04f;
    //    //    //std::cout << "windowSize.x:" << windowSize.x << " | windowSize.y:" << windowSize.y << "  || xNDC" << xNDC << " | yNDC " << yNDC << "  || maxdX " << maxXNDC << std::endl;
    //    //    delta.x = ComputeEaseExponent(glm::abs(delta.x)) * glm::sign(delta.x);
    //    //    delta.y = ComputeEaseExponent(glm::abs(delta.y)) * glm::sign(delta.y);
    //    //    if (_mouseStates[GLFW_MOUSE_BUTTON_RIGHT].buttonState == GLFW_REPEAT) {
    //    //        _camera.rotateCamera(delta * 15.0f);
    //    //    }
    //    //    else {
    //    //        glm::vec2 normDelta = glm::normalize(delta);
    //    //        glm::vec3 rightDir = -normDelta.x * _camera.transform.getRight();
    //    //        glm::vec3 upDir = normDelta.y * _camera.transform.getUp();
    //    //        _camera.moveCamera(glm::normalize(rightDir + upDir), 4.0f);
    //    //    }
    //    //    _prevCursorX = xpos;
    //    //    _prevCursorY = ypos;
    //    //}
    //}

    float ComputeEaseExponent(float input, float minInput = 0.0f, float maxInput = 1.0f, float lowerBound = 0.0f, float upperBound = 1.0f, float exponent = 4.0f) {
        float inputRange = glm::abs(maxInput - minInput);
        float outputRange = glm::abs(upperBound - lowerBound);
        return upperBound - glm::pow(1.0f + ((minInput - input) / inputRange), exponent) * outputRange;
    }

    

    void cleanup() {
        _window.cleanup();
        //glfwDestroyWindow(window);
        //glfwTerminate();
    }
};
}

int main(int argc, char* argv[]) {
    PenguinEngine::Time::initialize();
    PenguinEngine::Random::initialize();
    PenguinEngine::PenguinEngineApplication app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}