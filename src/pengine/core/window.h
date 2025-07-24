#pragma once
#ifndef WINDOW_H
#define WINDOW_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include "constants.h"

namespace penguin_engine {
    class Window {
    public:
        bool isInitialized = false;

        void initialize();

        void cleanup();

        struct SDL_Window* getSDLWindow() {
            return _window;
        }

        static int get_window_width() {
            return _width;
        }

        static int get_window_height() {
            return _height;
        }

        static float getWindowAspectRatio() {
            return  (float)_width / _height;
        }

    private:
        

        static struct SDL_Window* _window;

        static int _width;
        static int _height;
    };
}

#endif