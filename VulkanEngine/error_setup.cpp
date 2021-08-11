#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "error_setup.hpp"


VKAPI_ATTR VkBool32 VKAPI_CALL debug::debugCallback(
VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
VkDebugUtilsMessageTypeFlagsEXT messageType,
const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
void* pUserData) {

    //arrow is used in pointers of objects/structs
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}  

void debug::glfwErrorCallback(int code, const char* description) {
    std::cerr << "GLFW Error " << code << ": " << description << std::endl;
}

VkResult debug::CreateDebugUtilsMessengerEXT(VkInstance instance, 
const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
const VkAllocationCallbacks* pAllocator, 
VkDebugUtilsMessengerEXT* pDebugMessenger) {
    //basically casting the return of vkGetInstanceProcAddr to a pointer function of the function being loaded.
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    //check if function successfully loaded
    if (func != nullptr) {
        //why dont I need messengar?
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        //TODO: figure out best error message
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

}

void debug::DestroyDebugUtilsMessengerEXT(VkInstance instance,
VkDebugUtilsMessengerEXT pDebugMessenger,
const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

    if (func != nullptr) {
        func(instance, pDebugMessenger, pAllocator);
    }
}

