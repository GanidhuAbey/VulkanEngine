#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//#include "engine_graphics.hpp"

#include <stdexcept>
#include <iostream>
#include <vector>
#include <math.h>
#include <algorithm>
#include <list>

//100 mb of data per allocation
//TODO: need to create a information struct in the style of vulkan to let user manipulate this data
//VkDeviceSize allocationSize = 5e8;

namespace mem {
struct MaMemoryInfo {
    VkDeviceSize bufferSize;
    VkDeviceSize allocationSize;
    VkBufferUsageFlags bufferUsage;
    VkMemoryPropertyFlags memoryProperties;
    uint32_t queueFamilyIndexCount;

};

struct MaBufferCreateInfo {
    const void* pNext = nullptr;
    VkBufferCreateFlags flags = 0;
    VkDeviceSize size;
    VkBufferUsageFlags usage;
    VkSharingMode sharingMode;
    uint32_t queueFamilyIndexCount;
    const uint32_t* pQueueFamilyIndices;

    VkMemoryPropertyFlags memoryProperties;
};

struct Space {
    VkDeviceSize offset;
    VkDeviceSize size;
};

struct MaFreeMemoryInfo {
    VkDeviceSize deleteSize;
    VkDeviceSize deleteOffset;
};

struct MaMemory {
    VkBuffer buffer;
    VkImage image;
    VkDeviceMemory memoryHandle;
    VkDeviceSize offset;
    size_t deleteIndex;
    bool allocate;

    std::list<Space> locations;
};

struct MaMemoryData {
    VkDeviceMemory memoryHandle;
    VkDeviceSize offset;
    VkDeviceSize resourceSize;
    size_t offsetIndex;
};
void maCreateBuffer(VkPhysicalDevice physicalDevice, VkDevice device, MaBufferCreateInfo* pCreateInfo, MaMemory* memory);
void maCreateImage();
void maMapMemory(VkDevice device, VkDeviceSize dataSize, MaMemory* pMemory, void* data);
uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

void maCreateMemory(VkPhysicalDevice physicalDevice, VkDevice device, MaMemoryInfo* poolInfo, VkBuffer* buffer, MaMemory* maMemory);
void maAllocateMemory(VkDeviceSize allocationSize, MaMemory* pMemory);
void maFreeMemory(MaFreeMemoryInfo freeInfo, MaMemory* pMemory);
void maDestroyMemory(VkDevice device, MaMemory maMemory);

} // namespace mem
