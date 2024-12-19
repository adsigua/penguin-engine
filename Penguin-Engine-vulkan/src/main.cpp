#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <string.h>

class HelloTriangleApplication {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow* window;
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    VkInstance instance;

    std::vector<const char*> requiredExtensions;
    std::vector<VkExtensionProperties> supportedExtensions;

    void initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    }

    void initVulkan() {
        createInstance();
    }

    void createInstance() {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        InitExtensions();

        createInfo.enabledExtensionCount = (uint32_t)requiredExtensions.size();
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();
        
        /*AddIfSupportedExtension(
            VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME, 
            VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR, 
            &createInfo);*/
        
        PrintInstanceRequiredExtensions(createInfo);
        PrintSupportedExtensions();

        createInfo.enabledLayerCount = 0;

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

    void InitExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        for (uint32_t i = 0; i < glfwExtensionCount; i++) {
            requiredExtensions.emplace_back(glfwExtensions[i]);
        }

        //available extensions
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        supportedExtensions = std::vector<VkExtensionProperties>(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, supportedExtensions.data());

        int toAddExtensionCount = glfwExtensionCount - 1;
        while (toAddExtensionCount >= 0) {
            if (isExtensionSupported(requiredExtensions[toAddExtensionCount])) {
                toAddExtensionCount--;
            }
            else {
                std::cout << "Extension " << requiredExtensions[toAddExtensionCount] << " not supported!" << '\n';
                requiredExtensions.erase(requiredExtensions.begin() + toAddExtensionCount);
            }
        }
    }

    bool isExtensionSupported(const char* extensionName) {
        for (const auto& extension : supportedExtensions) {
            if (std::strcmp(extensionName, extension.extensionName)==0) {
                return true;
            }
        }
        return false;
    }

    void AddIfSupportedExtension(const char* extensionName, uint32_t extensionFlag, VkInstanceCreateInfo* createInfo)
    {
        if (isExtensionSupported(extensionName)) {
            requiredExtensions.emplace_back(extensionName);
            (*createInfo).flags |= extensionFlag;

            (*createInfo).enabledExtensionCount = (uint32_t)requiredExtensions.size();
            (*createInfo).ppEnabledExtensionNames = requiredExtensions.data();
        }
        else {
            std::cout << "Extension " << extensionName << " not supported!" << '\n';
        }
    }
    
    void PrintInstanceRequiredExtensions(VkInstanceCreateInfo createInfo) {
        std::cout << "available extensions:\n";

        for (uint32_t i = 0; i < createInfo.enabledExtensionCount; i++) {
            std::cout << '\t' << createInfo.ppEnabledExtensionNames[i] << '\n';
        }
    }

    void PrintSupportedExtensions() {
        std::cout << "supported extensions:\n";

        for (const auto& extension : supportedExtensions) {
            std::cout << '\t' << extension.extensionName << '\n';
        }
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    void cleanup() {
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }
};

int main() {
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