#include "window.h"

namespace PenguinEngine {
    void Window::initialize() {
        // We initialize SDL and create a window with it.
        SDL_Init(SDL_INIT_VIDEO);

        SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

        _window = SDL_CreateWindow(
            "Penguin Engine",
            DEFAULT_WINDOW_WIDTH,
            DEFAULT_WINDOW_HEIGHT,
            window_flags);

        _width = DEFAULT_WINDOW_WIDTH;
        _height = DEFAULT_WINDOW_HEIGHT;
        isInitialized = true;
    }

    void Window::cleanup() {
        if (isInitialized) {
            SDL_DestroyWindow(_window);
        }
    }

    struct SDL_Window* Window::_window = {nullptr};
    int Window::_width = DEFAULT_WINDOW_WIDTH;
    int Window::_height = DEFAULT_WINDOW_HEIGHT;
}

