#pragma once
#ifndef PENGINE_CORE_WINDOW_H
#define PENGINE_CORE_WINDOW_H

#include <SDL3/SDL.h>

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