#include "swapchain_support.hpp"

namespace create {
    SwapChainSupport::SwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
        querySwapChainSupport(device, surface);
    }

    void SwapChainSupport::querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
        //get surface capabilities
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);

        //get formats
        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount > 0) {
            formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, formats.data());
        }
        //get presentation modes
        uint32_t modeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &modeCount, nullptr);

        if (modeCount > 0) {
            presentModes.resize(modeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &modeCount, presentModes.data());
        }
    }
}