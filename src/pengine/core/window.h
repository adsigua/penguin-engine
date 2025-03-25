#pragma once
#ifndef WINDOW_H
#define WINDOW_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <pengine/core/constants.h>

namespace PenguinEngine {
    class Window {
    public:
        bool isInitialized = false;

        void initialize();

        void cleanup();

        struct SDL_Window* getWindow() {
            return _window;
        }

        static int getWindowWidth() {
            return _width;
        }

        static int getWidowHeight() {
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