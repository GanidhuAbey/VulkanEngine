#include "memory_allocator.hpp"

#include <bitset>
#include <cstring>
using namespace mem;

uint32_t mem::findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    //uint32_t suitableMemoryForBuffer = 0;
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if (typeFilter & (1 << i) && ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)) {
            return i;
        }
    }

    throw std::runtime_error("could not find appropriate memory type");

}
//PURPOSE - create a buffer and allocate it with the memory required by the user
//PARAMETERS - [VkPhysicalDevice] physicalDevice - the GPU the renderer is rendering on
//           - [VkDevice] device - the logical device containing info on which parts of the GPU i'll be using
//           - [MaBufferCreateInfo*] pCreateInfo - information struct needed to create a buffer
//           - [MaMemory*] memory - the memory struct this function will write its data to
//RETURNS - NONE
void mem::maCreateBuffer(VkPhysicalDevice physicalDevice, VkDevice device, MaBufferCreateInfo* pCreateInfo, MaMemory* pMemory) {
    VkBufferCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.pNext = pCreateInfo->pNext;
    createInfo.flags = pCreateInfo->flags;
    createInfo.usage = pCreateInfo->usage;
    createInfo.size = pCreateInfo->size;
    createInfo.sharingMode = pCreateInfo->sharingMode;
    createInfo.queueFamilyIndexCount = pCreateInfo->queueFamilyIndexCount;
    createInfo.pQueueFamilyIndices = pCreateInfo->pQueueFamilyIndices;

    if (vkCreateBuffer(device, &createInfo, nullptr, &pMemory->buffer) != VK_SUCCESS) {
        throw std::runtime_error("could not create buffer");
    };

    //allocate desired memory to buffer
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, pMemory->buffer, &memRequirements);

    //allocate memory for buffer
    VkMemoryAllocateInfo memoryInfo{};
    memoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryInfo.allocationSize = memRequirements.size;
    //too lazy to even check if this exists will do later TODO
    memoryInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, pCreateInfo->memoryProperties);

    VkResult allocResult = vkAllocateMemory(device, &memoryInfo, nullptr, &pMemory->memoryHandle);

    if (allocResult != VK_SUCCESS) {
        throw std::runtime_error("could not allocate memory for memory pool");
    }

    if (vkBindBufferMemory(device, pMemory->buffer, pMemory->memoryHandle, 0) != VK_SUCCESS) {
        throw std::runtime_error("could not bind allocated memory to buffer");
    }

    //initialize the memory chunk for the maAllocateMemory to use
    Space freeSpace{};
    freeSpace.offset = 0;
    freeSpace.size = pCreateInfo->size;

    pMemory->locations.push_back(freeSpace);
}
//PURPOSE - create a image and allocate it with the memory required by the user
void mem::maCreateImage()  {}


//PURPOSE - given a memory block preserve a chunk of memory to be 'allocated' so that it can be filled with data without causing
//          conflicts
//PARAMETRS - [VkDeviceSize] allocationSize (how much memory needs to be preserved in bytes)
//            [MaMemory*] pMemory (memory struct which we will pass on the data for where the memory has been preserved)
//RETURNS - NONE
void mem::maAllocateMemory(VkDeviceSize allocationSize, MaMemory* pMemory) {
    for (auto it = pMemory->locations.rbegin(); it != pMemory->locations.rend(); it++) {
        //printf("allocation size: %zu", it->size);
        if (it->size >= allocationSize) {
            //printf("allocation spot size: %zu, allocation size: %zu \n", it->size, allocationSize);
            VkDeviceSize offsetLocation;
            memcpy(&offsetLocation, &(it->offset), sizeof(it->offset));
            pMemory->offset = offsetLocation;
            pMemory->allocate = true;

            it->offset = it->offset + allocationSize;
            it->size = it->size - allocationSize;

            //printf("allocation size afterwards: %zu", it->size);
            break;
        }
    }

    if (!pMemory->allocate) {
        throw std::runtime_error("welp now u have to fix this");
    }
}

//PURPOSE - free memory described by the struct the user gives so that future memory allocations can use the space to allocate memory
//PARAMETERS - [MaFreeMemoryInfo] freeInfo (struct describing the data the user wants to  free)
//           - [MaMemory*] pMemory (pointer to the memory where the data is located)
//RETURNS - NONE
void mem::maFreeMemory(MaFreeMemoryInfo freeInfo, MaMemory* pMemory) {
    //need to do a check on where this data is
    //iterate through the memory locations to figure out if any offsets match
    for (auto it = pMemory->locations.begin(); it != pMemory->locations.end(); it++) {
        VkDeviceSize freeOffset = freeInfo.deleteOffset + freeInfo.deleteSize;
        if (freeOffset < it->offset) {
            //we know we need to insert a new value into the vector, so we can break after we finish
            const Space& freeSpace = {
                freeInfo.deleteOffset,
                freeInfo.deleteSize,
            };

            //insert into list
            pMemory->locations.insert(it, freeSpace);
            break;
        }
        if (freeOffset == it->offset) {
            //we know we need to move the value in the vector, so we can break after we finish
            it->offset -= freeInfo.deleteSize;
            it->size += freeInfo.deleteSize;
            break;
        }
    }
}

//PURPOSE - map the given data to the given memory
//PARAMETERS - [VkDevice] device - the logical device
//           - [VkDeviceSize] dataSize - the size of the data being mapped
//           - [MaMemory*] pMemory - pointer to memory struct
//           - [void*] data - the data actually being mapped
void mem::maMapMemory(VkDevice device, VkDeviceSize dataSize, MaMemory* pMemory, void* data) {
    if (!pMemory->allocate) {
        throw std::runtime_error("tried to map memory to unallocated data");
    }
    pMemory->allocate = false;

    void* pData;
    if (vkMapMemory(device, pMemory->memoryHandle, pMemory->offset, dataSize, 0, &pData) != VK_SUCCESS) {
        throw std::runtime_error("could not map data to memory");
    }
    memcpy(pData, data, dataSize);
    vkUnmapMemory(device, pMemory->memoryHandle);
}

void mem::maDestroyMemory(VkDevice device, MaMemory maMemory) {
    vkDeviceWaitIdle(device);
    //free the large chunk of memory
    vkFreeMemory(device, maMemory.memoryHandle, nullptr);

    //destroy the associated buffer
    vkDestroyBuffer(device, maMemory.buffer, nullptr);
}
