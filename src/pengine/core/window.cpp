#include "window.h"

namespace penguin_engine {
    void Window::initialize() {
        // We initialize SDL and create a window with it.
        SDL_Init(SDL_INIT_VIDEO);

        SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

        _window = SDL_CreateWindow(
            "Penguin Engine",
            constants::DEFAULT_WINDOW_WIDTH,
            constants::DEFAULT_WINDOW_HEIGHT,
            window_flags);

        _width = constants::DEFAULT_WINDOW_WIDTH;
        _height = constants::DEFAULT_WINDOW_HEIGHT;
        isInitialized = true;
    }

    void Window::cleanup() {
        if (isInitialized) {
            SDL_DestroyWindow(_window);
        }
    }

    struct SDL_Window* Window::_window = {nullptr};
    int Window::_width = constants::DEFAULT_WINDOW_WIDTH;
    int Window::_height = constants::DEFAULT_WINDOW_HEIGHT;
}

