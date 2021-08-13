#define VERSION_1 0;

#include "engine_init.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//this is the standard validation layer that is included in the vulkan sdk
//if more validation layers are needed then its best to add them here
//(i think)
const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

using namespace core;

bool EngineInit::checkValidationLayerSupport()
{
    uint32_t layerCount = 0;

    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName : validationLayers)
    {
        bool layerFound = false;
        for (const auto &availableLayer : availableLayers)
        {
            if (strcmp(layerName, availableLayer.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }
        if (!layerFound)
        {
            return false;
        }
    }
    return true;
}

//checks if the needed device extensions are supported
bool EngineInit::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t deviceExtCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &deviceExtCount, nullptr);

    if (deviceExtCount == 0)
    {
        return false;
    }

    std::vector<VkExtensionProperties> supportedDeviceExtensions(deviceExtCount);

    vkEnumerateDeviceExtensionProperties(device, nullptr, &deviceExtCount, supportedDeviceExtensions.data());

    for (const auto &deviceExt1 : deviceExtensions)
    {
        bool found = false;
        for (const auto &deviceExt2 : supportedDeviceExtensions)
        {
            if (strcmp(deviceExt1, deviceExt2.extensionName))
            {
                found = true;
            }
        }
        if (!found)
        {
            return false;
        }
    }
    return true;
}

//TODO: update to vulkan version 1.2
int EngineInit::rateDeviceSuitability(VkPhysicalDevice device)
{
    int score = 0;

    //grab data of device
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;

    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    VkPhysicalDeviceVulkan12Features vulkan12Features{};
    vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    //vulkan12Features.separateDepthStencilLayouts = VK_TRUE;

    //VkPhysicalDeviceFeatures2 deviceFeatures{};
    //deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    //deviceFeatures.pNext = &vulkan12Features;
    //eviceFeatures.features = vulkan10Features;

    //vkGetPhysicalDeviceFeatures2(device, &deviceFeatures);


    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    }

    //NOTE: their is also a different limit for 3D image dimensions
    score += deviceProperties.limits.maxImageDimension2D;

    //needed to draw images.
    if (!deviceFeatures.geometryShader) {
        return 0;
    }

    //check if neede7d extensions are supported by device
    if (!checkDeviceExtensionSupport(device)) {
        return 0;
    }

    QueueData indices(device, surface);

    //the '.' operator is used when dealing with the actual object not a pointer to it.
    //can use the isComplete() function but want to keep the above note
    if (!indices.isComplete())
    {
        return 0;
    }

    //check if the binding count is atleast 1
    //TODO: this is a kind of check you do when you know what you'll be drawing,
    //but we should figure out some general parameters to make sure the device we pull can draw at most things
    if (deviceProperties.limits.maxVertexInputBindings < 1 || deviceProperties.limits.maxVertexInputBindingStride < sizeof(data::Vertex))
    {
        return 0;
    }
    score += 10 * deviceProperties.limits.maxVertexInputBindings;

    //check swapchain support
    bool extensionsSupported = checkDeviceExtensionSupport(device);
    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupport details(device, surface);
        swapChainAdequate = !details.formats.empty() && !details.presentModes.empty();
    }

    if (!extensionsSupported || !swapChainAdequate)
    {
        return 0;
    }

    //check if seperate depth/stencil passes are acceptable

    return score;
}

std::vector<const char *> EngineInit::getRequiredExtensions()
{
    uint32_t glfwExtensionsCount = 0;
    const char **glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

    //Now add a function setupDebugMessenger to be called from initVulkan right after createInstance:const char** to  std::vector<const char*>
    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);

    //check if debug is enabled
    if (enableValidationLayers)
    {
        //add VK_EXT_debug_utils
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

//challenge function to compare extensions returned by glfw versus the supported extensions
bool EngineInit::checkSupportedExtensions(std::vector<const char *> requiredExtensions)
{
    //grab the number of supported extensions
    uint32_t extensionCount = 0;
    //do layers have extensions associated with it?
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    //create vector to hold all supported extension properties
    std::vector<VkExtensionProperties> supportedExtensions(extensionCount);

    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, supportedExtensions.data());

    //generally the amounted of extensions we're dealing with is small so it should be fine to use a double iteration
    uint32_t includedExtensions = 0;
    for (const auto &requiredExtension : requiredExtensions)
    {
        bool extensionFound = false;
        for (const auto &extension : supportedExtensions)
        {
            if (strcmp(requiredExtension, extension.extensionName) == 0)
            {
                extensionFound = true;
            }
        }
        if (!extensionFound)
        {
            return false;
        }
    }

    return true;
}

//the '&' in function arguements acts largely like a '*' except its entry cannot be null
void EngineInit::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debug::debugCallback;
}

void EngineInit::initialize(GLFWwindow *userWindow)
{
    window = userWindow;

    createInstance();
    createSurface();
    setupDebugMessenger();
    pickPhysicalDevice();
    createLogicalDevice();
    createCommandPool();
}

EngineInit::~EngineInit()
{
    vkDeviceWaitIdle(device);

    std::cout << "engine instance destruction.." << std::endl;

    if (enableValidationLayers)
    {
        debug::DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    vkDestroyCommandPool(device, commandPool, nullptr);

    vkDestroyDevice(device, nullptr);

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
}

void EngineInit::setupDebugMessenger()
{
    if (!enableValidationLayers)
        return;

    //fill in object struct
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};

    populateDebugMessengerCreateInfo(createInfo);

    //functions that are part of extensions (like this one) are  not not loaded automatically.
    //inorder to load functions like these we have to use vkGetInstanceProcAddr
    //create object
    if (debug::CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("could not create debug messenger");
    }
}

void EngineInit::createInstance()
{
    //check if debug mode is on and if user device has accessed to the validation layers they set in validationLayers
    if (enableValidationLayers && !checkValidationLayerSupport())
    {
        throw std::runtime_error("the requested validation layers are not available!");
    }

    //fill struct with data on application
    //helps graphics card optimize by knowing more about the application
    VkApplicationInfo appInfo{};

    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";

    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    //this will create a vulkan 1.0 version which is used in the tutorial.
    //TODO: try a higher version of vulkan to see how the program changes
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    std::vector<const char *> extensions = getRequiredExtensions();

    //TODO: move all this into the checkSupportedExtension()
    //check supported extensions
    if (!checkSupportedExtensions(extensions))
    {
        throw std::runtime_error("required vulkan extensions are not supported!");
    }

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    //if not place inside the if statement, this debug info will run out of scope and be freed from memory, when it
    //is still needed to be able to create an instance.
    VkDebugUtilsMessengerCreateInfoEXT createDebugInfo{};

    //setup debug layers and extensions for instance#define VK_USE_PLATFORM_WIN32_KHR
    if (enableValidationLayers)
    {
        //convert size int to uint32_t
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(createDebugInfo);
        //do you need to do this?
        //TODO: see how removing this cast changes things once we draw triangle
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&createDebugInfo;
    }
    //don't need to do this but it will probably be useful to be as descriptive as possible
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    //TODO: "pAllocator is not NULL" not sure if possible to make parameter nullptr
    //in future vulkan versions
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create instance");
    }
}
void EngineInit::createSurface()
{
    //glfw can do all this for me plus all other platforms and i won't to decide between anything

    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("surface could not be created");
    }
}
void EngineInit::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("no GPU's on this machine supports vulkan");
    }

    //makes int the key and VkPhysicalDevice the value, multimap is like a dictionary with a key-value pair, but multimap also keeps the values sorted by the keys.
    std::multimap<int, VkPhysicalDevice> candidates;

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto &device : devices)
    {
        int score = rateDeviceSuitability(device);
        candidates.insert(std::make_pair(score, device));
    }

    if (candidates.rbegin()->first > 0)
    {
        //this device is suitable
        physicalDevice = candidates.rbegin()->second;
        VkPhysicalDeviceProperties deviceProperties{};

        vkGetPhysicalDeviceProperties(candidates.rbegin()->second, &deviceProperties);

        std::cout << deviceProperties.deviceName << std::endl;
    }
    else
    {
        throw std::runtime_error("could not find a suitable GPU");
    }

    //findQueueFamilies(physicalDevice);
}
void EngineInit::createLogicalDevice()
{
    QueueData indices(physicalDevice, surface);

    std::cout << "graphics queue: " << indices.graphicsFamily.value() << std::endl;
    std::cout << "present queue: " << indices.presentFamily.value() << std::endl;

    //Load Queues
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> queueIndices = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    //iterate through queuefamilies and fill in struct data
    float queuePriority = 1.0f;
    for (uint32_t i : queueIndices)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = i;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfos.push_back(queueCreateInfo);
    }

    //this defined a struct of features that we can use, right now its set to all VK_FALSE meaning no features are needed
    //later on we will need geometry shaders from these device features (which is why we already queried for it in the pickPhysicalDevice() option)
    VkPhysicalDeviceFeatures deviceFeatures{};

    //TODO: move this somehwere else
    VkPhysicalDeviceProperties deviceProperties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

    //filling in the struct for the logical device
    VkDeviceCreateInfo createLInfo{};
    createLInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createLInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    //documentation writes this as an array but when only dealing with a single queue, you can just put a single value and its fine.
    createLInfo.pQueueCreateInfos = queueCreateInfos.data();
    createLInfo.pEnabledFeatures = &deviceFeatures;

    //NOTE: creating logical devices also allows us to enable extensions to extend the functionality of Vulkan
    //examples of this would be VK_KHR_swapchain which is a device specific extension designed to present rendered images to the screen.

    //this part is not required for newer versions of vulkan, but older versions still rely on this
    if (enableValidationLayers)
    {
        //set up device layers for this logical device
        createLInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createLInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createLInfo.enabledLayerCount = 0;
    }

    //enable VK_KHR_SWAPCHAIN extension
    createLInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createLInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (vkCreateDevice(physicalDevice, &createLInfo, nullptr, &device) != VK_SUCCESS)
    {
        throw std::runtime_error("could not create logical device");
    }

    //TODO: not sure where to put this...
    //grab queue from device
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

void EngineInit::createCommandPool()
{
    QueueData queueFamilyIndices(physicalDevice, surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = 0; // Optional

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("could not create command pool");
    }
}
