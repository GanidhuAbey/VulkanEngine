#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

namespace create {
class SwapChainSupport {
    public:
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;

    public:
        SwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
    
    private:
        void querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
};
}