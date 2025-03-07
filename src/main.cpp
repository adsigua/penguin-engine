#include <stdlib.h>     /* srand, rand */

#include <vector>
#include <iostream>
#include <unordered_map>
#include <cmath>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/string_cast.hpp>

#include "TransformObject.h"
#include "Transform.h"
#include "VKEngine.h"
#include "Time.h"

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

class HelloTriangleApplication {
public:
    /*HelloTriangleApplication() {
        
    }*/

    void run() {
        initWindow();
        _renderer.InitVulkan(window);
        createObjects();
        mainLoop();
        cleanup();
    }

    void handleInput(int key, int scancode, int action, int mods) {
        if (_keyStates.find(key) != _keyStates.end()) {
            if (action == GLFW_PRESS) {
                _keyStates[key] = true;
            }
            else if (action == GLFW_RELEASE) {
                _keyStates[key] = false;
            }
        }

        if (key == GLFW_KEY_R && action == GLFW_PRESS) {
            _camera.ResetCamera(false);
            std::cout << "rot cam " << GetMatrixString(_camera.transform.getLocalToWorldMatrix()) << std::endl;
        }
    }

    void handleMouseInput(int button, int action, int mods)
    {
        if (_mouseStates.find(button) != _mouseStates.end()) {
            if (action == GLFW_PRESS) {
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);
                _mouseStates[button].startPosition = glm::vec2(xpos, ypos);
                _mouseStates[button].buttonState = GLFW_PRESS;
            }
            else if (action == GLFW_RELEASE) {
                _mouseStates[button].buttonState = GLFW_RELEASE;
            }
        }
    }

private:
    GLFWwindow* window;
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;
    PenguinEngine::Graphics::VKEngine _renderer;

    std::vector<RenderObject> _renderedObjects;

    const uint16_t SPAWN_COUNT = 1;
    const float SPAWN_SIZE = 3.0f;
    const float CAMERA_DISTANCE = 5.0f;

    const float CAMERA_MOVE_SPEED = 4.0f;
    const float CAMERA_FAST_SPEED_MULTIPLIER = 2.0f;
    const float CAMERA_TURN_SPEED = 70.0f;
    const float CAMERA_TURN_MIN_SCREEN_DELTA = 5.0f;

    std::unordered_map<int, bool> _keyStates;
    std::unordered_map<int, MouseInput> _mouseStates;

    double _prevCursorX, _prevCursorY;
    glm::vec2 _prevCursorPos;
    glm::vec3 _prevCursorVPos;

    glm::vec2 windowSize;

    Camera _camera;

    TimeDelayer _rightClickChecker;
    //RenderObject squareObject;
    //std::vector<PenguinEngine::Graphics::SwapChainData> _swapChainData;

    void initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        
        initInputStates();

        int width, height;
        glfwGetWindowSize(window, &width, &height);
        windowSize = glm::vec2(width, height);

        glm::mat4 someMat = glm::mat4(1.0f);
        std::cout << "mat" << glm::to_string(someMat) << std::endl;
        someMat = glm::translate(someMat, glm::vec3(1, 2, 3));
        std::cout << "mat" << glm::to_string(someMat) << std::endl;
    }

    void initInputStates() {
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
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
        app->_renderer.SetFrameBufferResized(true);
        app->windowSize = glm::vec2(width, height);
    }

    static void inputCallBack(GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
        app->handleInput(key, scancode, action, mods);
    }

    static void mouseCallBack(GLFWwindow* window, int button, int action, int mods) {
        auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
        app->handleMouseInput(button, action, mods);
    }

    void createObjects() {
        glm::mat4 mat4test = glm::mat4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
        glm::vec4 vec0 = mat4test[0];
        glm::vec4 vec1 = mat4test[1];
        glm::vec4 vec2 = mat4test[2];
        glm::vec4 vec3 = mat4test[3];

        std::cout << "mat4test: " << GetMatrixString(mat4test) << std::endl;
        std::cout << "vec0:" << glm::to_string(vec0) << std::endl;
        std::cout << "vec1:" << glm::to_string(vec1) << std::endl;
        std::cout << "vec2:" << glm::to_string(vec2) << std::endl;
        std::cout << "vec3:" << glm::to_string(vec3) << std::endl << std::endl;


        _camera = Camera(75.0f, _renderer.GetSwapChainAspectRatio(), 0.1f, 200.0f);
        //_camera.transform.SetRotation_Matrix(glm::mat4(1.0f));
        _camera.transform.LookAt(glm::vec3(0.0f, 0.0f, CAMERA_DISTANCE), glm::vec3(0, 0, 0), glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)));

        _renderedObjects.resize(SPAWN_COUNT);
        for (int i = 0; i < _renderedObjects.size(); i++) {
            float xPos = (rand() % 100) / 100.0f, yPos = (rand() % 100) / 100.0f, zPos = (rand() % 100) / 100.0f;
            xPos = xPos - 0.5f;
            yPos = yPos - 0.5f;
            RenderObject renderObj = RenderObject();
            glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(xPos * SPAWN_SIZE, yPos * 0.5f * SPAWN_SIZE, -zPos * SPAWN_SIZE));
            glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(20.0f * (rand() % 100) / 100.0f * 360.0f), glm::vec3(0.0, 1.0, 0.0));
            renderObj.transform.setLocalToWorldMatrix(translation * rotation * scale);
            _renderedObjects[i] = renderObj;
        }

    }

    void updateObjects() {
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - startTime).count();
        int val = (int)(glm::round(time) / 0.3f);
 
        for (int i = 0; i < _renderedObjects.size(); i++) {
            //_renderedObjects[i].transform.Rotate(glm::radians(5.0f) * PenguinEngine::Time::getDeltaTime(), glm::vec3(0.0, 1.0, 0.0));
        }
        //_camera.transform.Rotate(glm::radians(5.0f) * PenguinEngine::Time::getDeltaTime(), glm::vec3(0.0, 1.0, 0.0));
        
        if ((int)glm::floor(time) % 2000 == 0) {
            //std::cout << "fr:" << time << " euler " << glm::to_string(glm::eulerAngles(_renderedObjects[0].transform.GetRotationQuat())) << std::endl;
            //std::cout << "rot cam " << GetMatrixString(_renderedObjects[0].transform.GetRotationMatrix()) << std::endl << std::endl;
        }

        _camera.aspectRatio = _renderer.GetSwapChainAspectRatio();
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            PenguinEngine::Time::Tick();
            checkForInput();
            updateObjects();
            _renderer.DrawFrame(_camera, &_renderedObjects);
        }
        _renderer.WaitRendererIdle();
    }

    void checkForInput() {
        glfwPollEvents();

        checkKeyboardInput();
        checkMouseInput();
    }

    void checkKeyboardInput() {
        float speedMult = _keyStates[GLFW_KEY_LEFT_SHIFT] ? CAMERA_FAST_SPEED_MULTIPLIER : 1.0f;
        

        if (_keyStates[GLFW_KEY_W]) {
            moveCamera(_camera.transform.getForward(), speedMult);
        }
        if (_keyStates[GLFW_KEY_A]) {
            moveCamera(_camera.transform.getRight(), speedMult);
        }
        if (_keyStates[GLFW_KEY_S]) {
            moveCamera(-_camera.transform.getForward(), speedMult);
        }
        if (_keyStates[GLFW_KEY_D]) {
            moveCamera(-_camera.transform.getRight(), speedMult);
        }
        if (_keyStates[GLFW_KEY_SPACE]) {
            moveCamera(_camera.transform.getUp(), speedMult);
        }
        if (_keyStates[GLFW_KEY_LEFT_CONTROL]) {
            moveCamera(-_camera.transform.getUp(), speedMult);
        }

        glm::vec2 rotateDir = glm::vec2(0.0f, 0.0f);
        if (_keyStates[GLFW_KEY_LEFT]) {
            rotateDir.x = 1;
        }
        else if (_keyStates[GLFW_KEY_RIGHT]) {
            rotateDir.x = -1;
        }
        else if (_keyStates[GLFW_KEY_UP]) {
            rotateDir.y = 1;
        }
        else if (_keyStates[GLFW_KEY_DOWN]) {
            rotateDir.y = -1;
        }
        if (rotateDir.x != 0 || rotateDir.y != 0) {
            rotateCamera(rotateDir);
        }
    }

    void checkMouseInput() {
        float currTime = PenguinEngine::Time::getTime();
        if (currTime < _rightClickChecker.nextTime) {
            //return;
            /*_rightClickChecker.nextTime = currTime + _rightClickChecker.requiredTime;
            std::cout << "screen pos: " << xpos << "," << ypos << std::endl;*/
        }

        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        if (_mouseStates[GLFW_MOUSE_BUTTON_RIGHT].buttonState != GLFW_RELEASE) {
            if (_mouseStates[GLFW_MOUSE_BUTTON_RIGHT].buttonState == GLFW_PRESS) {
                _prevCursorX = xpos;
                _prevCursorY = ypos;

                _mouseStates[GLFW_MOUSE_BUTTON_RIGHT].buttonState = GLFW_REPEAT;
            } else if (std::abs(xpos - _prevCursorX) >= CAMERA_TURN_MIN_SCREEN_DELTA || std::abs(ypos - _prevCursorY) >= CAMERA_TURN_MIN_SCREEN_DELTA) {
               
                float dX = xpos - _prevCursorX;
                float dY = ypos - _prevCursorY;

                float xNDC = (windowSize.x * 0.5f + dX);
                float yNDC = (windowSize.y * 0.5f + dY);
                glm::vec3 deltaPos = getScreenSpaceViewPos(xpos, ypos);

                glm::vec3 camForwardPos = _camera.transform.GetPosition() + _camera.transform.getForward();
                glm::vec3 camNewForwardPos = _camera.transform.GetPosition() + deltaPos;
                glm::mat4 rotation = glm::lookAt(_camera.transform.GetPosition(), camNewForwardPos, glm::vec3(0.0f, 1.0f, 0.0f));
                
                _prevCursorX = xpos;
                _prevCursorY = ypos;

                glm::vec3 screenWorldPos = _camera.transform.getLocalToWorldMatrix() * getScreenSpaceWorldPos(xpos, ypos);
                _renderedObjects[0].transform.SetPosition( screenWorldPos);
                std::cout << "screenWorldPos: <" << glm::to_string(screenWorldPos) << std::endl;

            }

            _rightClickChecker.nextTime = currTime + _rightClickChecker.requiredTime;
        }
    }

    glm::vec4 getScreenSpaceViewPos(double x, double y) {
        float xNdc0 = (2.0f * x / windowSize.x) - 1.0f, yNdc0 = 1.0f - (2.0f * y / windowSize.y);
        glm::vec4 csPos = glm::vec4(xNdc0, yNdc0, -1.0f, 1.0f);
        glm::mat4 invProj = glm::inverse(_camera.GetProjectionMatrix(false));
        glm::vec4 vPos = (invProj * csPos) / csPos.w;

        return vPos;
    }

    glm::vec4 getScreenSpaceWorldPos(double x, double y) {
        float xNdc0 = (2.0f * x / windowSize.x) - 1.0f, yNdc0 = 1.0f - (2.0f * y / windowSize.y);
        glm::vec4 csPos = glm::vec4(xNdc0, yNdc0, -1.0f, 1.0f);
        glm::mat4 invProj = glm::inverse(_camera.GetProjectionMatrix(false));
        glm::vec4 vPos = (invProj * csPos) / csPos.w;

        return vPos;
    }

    glm::vec4 getScreenSpaceViewPos_NDC(double x, double y) {
        glm::vec4 csPos = glm::vec4(x, y, -1.0f, 1.0f);
        glm::mat4 invProj = glm::inverse(_camera.GetProjectionMatrix(false));
        glm::vec4 vPos = (invProj * csPos) / csPos.w;

        return vPos;
    }

    void moveCamera(glm::vec3 dir, float speedMult) {
       
        glm::vec3 camPos = _camera.transform.GetPosition();
        camPos += dir * CAMERA_MOVE_SPEED * speedMult * PenguinEngine::Time::getDeltaTime();
        _camera.transform.SetPosition(camPos);
        std::cout << "model cam:" << GetMatrixString(_camera.transform.getLocalToWorldMatrix()) << std::endl << std::endl;

        //std::cout << "cam pos: <" << camPos.x << ", " << camPos.y << ", " << camPos.z << ">" << std::endl;
    }

    void rotateCamera(glm::vec2 dir) {

        glm::vec3 camEuler = glm::eulerAngles(_camera.transform.GetRotationQuat());
        float turnAngle = glm::radians(CAMERA_TURN_SPEED) * PenguinEngine::Time::getDeltaTime();
        if (dir.x != 0) {
            _camera.transform.Rotate(turnAngle * dir.x, glm::vec3(0.0f, 1.0f, 0.0f));
            //std::cout << "rot cam" << turnAngle * dir.x << " euler y" << glm::degrees(camEuler.y) << std::endl;
        }
        if (dir.y != 0) {
            _camera.transform.Rotate(turnAngle * dir.y, _camera.transform.getRight());
        }
        //std::cout << "forward " << glm::to_string(_camera.transform.getForward()) << "  euler" << glm::to_string(camEuler) << std::endl;
        std::cout << "model cam:" << GetMatrixString(_camera.transform.getLocalToWorldMatrix()) << std::endl << std::endl;
    }


    std::string GetMatrixString(glm::mat4 mat) {
        return
            "\n{ " + std::to_string(mat[0][0]) + ", " + std::to_string(mat[1][0]) + ", " + std::to_string(mat[2][0]) + ", " + std::to_string(mat[3][0]) + "\n" +
            "  " + std::to_string(mat[0][1]) + ", " + std::to_string(mat[1][1]) + ", " + std::to_string(mat[2][1]) + ", " + std::to_string(mat[3][1]) + "\n" +
            "  " + std::to_string(mat[0][2]) + ", " + std::to_string(mat[1][2]) + ", " + std::to_string(mat[2][2]) + ", " + std::to_string(mat[3][2]) + "\n" +
            "  " + std::to_string(mat[0][3]) + ", " + std::to_string(mat[1][3]) + ", " + std::to_string(mat[2][3]) + ", " + std::to_string(mat[3][3]) + " }";
    }

    void cleanup() {
        _renderer.Cleanup();
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