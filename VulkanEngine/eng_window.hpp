#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//#include "engine_init.hpp"
#include "error_setup.hpp"
#include "common.hpp"

namespace core {

class EngWindow {
    //construct/deconstruct functions
    public:
        GLFWwindow* initWindow(int w, int h, const char* name);
        ~EngWindow();

    //private functions for library
    private:
        GLFWwindow *window;
        int width;
        int height;
        const char* title;
    //functions user can interact with
    public:
        bool closeRequest();
        bool isKeyPressed(WindowInput input);

        void getCursorPosition(double* xPos, double* yPos);
        void captureCursor();
};

}
