#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <optional>
#include <iostream>

namespace create {
class QueueData {
    public:
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

    public:
        QueueData(VkPhysicalDevice device, VkSurfaceKHR surface);
    public:
        bool isComplete() {return graphicsFamily.has_value() && presentFamily.has_value();}

    private: 
        void findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
};
}