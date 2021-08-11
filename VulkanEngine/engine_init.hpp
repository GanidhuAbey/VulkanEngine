#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "eng_window.hpp"
#include "queue.hpp"
#include "swapchain_support.hpp"
#include "error_setup.hpp"
#include "data_formats.hpp"

#include "glm/glm.hpp"

#include <iostream>
#include <vector>
#include <string.h>
#include <stdexcept>
#include <set>
#include <map>

namespace create {

class EngineInit {
    public:
        VkSurfaceKHR surface;
        VkDevice device;
        VkPhysicalDevice physicalDevice;
        GLFWwindow* window;
        VkCommandPool commandPool;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
    private:
        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;

    public:
        void initialize(GLFWwindow* userWindow);
        ~EngineInit();

    private:
        void createInstance();
        void createSurface();
        void setupDebugMessenger();
        void pickPhysicalDevice();
        void createLogicalDevice();
        void createCommandPool();
        int rateDeviceSuitability(VkPhysicalDevice device);
        bool checkValidationLayerSupport();
        std::vector<const char*> getRequiredExtensions();
        bool checkSupportedExtensions(std::vector<const char*> requiredExtensions);
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    public:
        std::vector<char> readFile(const std::string& fileName);
};

}
