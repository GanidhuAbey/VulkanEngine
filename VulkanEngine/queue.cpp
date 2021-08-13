#include "queue.hpp"

//TODO: move this into a header file, too small and waste compile time

using namespace core;
QueueData::QueueData(VkPhysicalDevice device, VkSurfaceKHR surface) {
    findQueueFamilies(device, surface);
}
void QueueData::findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
    //logic to fill indices struct up
    uint32_t queueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, queueFamilies.data());

    //NOTE: the reason for the const auto& is to specify that the data being read from the for loop is NOT being modified
    int i = 0;

    for (const auto& queue : queueFamilies) {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport) {
            presentFamily = i;
        }
        if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsFamily = i;
        }
        if (isComplete()) {
            break;
        }
        i++;
    }
}
