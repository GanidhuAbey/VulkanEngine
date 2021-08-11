
//Sorry GLFW, this entire file is basically a copy paste of your stuff :(

#include "eng_window.hpp"
#include "common.hpp"


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

using namespace create;

EngWindow::~EngWindow() {
    std::cout << "window destruction..." << std::endl;
    glfwDestroyWindow(window);
    glfwTerminate();
}

GLFWwindow* EngWindow::initWindow(int w, int h, const char* name) {
    width = w;
    height = h;
    title = name;

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    
    if (enableValidationLayers) {
        glfwSetErrorCallback(debug::glfwErrorCallback);
    }

    //create window
    window = glfwCreateWindow(width, height, title, nullptr,  nullptr);

    return window;
}

//=======================================================================
//user interact functions
//closeRequest() - returns true when user makes request to close window
//                 abstraction of the glfw functions for user.
//parameters:
//  NONE
//returns : bool - user window close request
bool EngWindow::closeRequest() {
    glfwPollEvents();

    return glfwWindowShouldClose(window);
}


/// - PURPOSE -
/// returns true if 'input' key has been pressed
/// - PARAMETERS - 
/// [WindowInput] input - key the user is checking
/// - RETURNS -
/// [bool] true when key is pressed 
bool EngWindow::isKeyPressed(WindowInput input) {
    int state = glfwGetKey(window, input);
    if (state == GLFW_PRESS) {
        return true;
    }
    return false;
}

void EngWindow::getCursorPosition(double* xPos, double* yPos) {
    glfwGetCursorPos(window, xPos, yPos);
}

void EngWindow::captureCursor() {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}