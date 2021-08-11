#include "engine.hpp"
#include "common.hpp"

using namespace create;

//define some global variables.
uint16_t indexCount = 0;
uint16_t vertexCount = 0;
uint16_t objectCount = 0;

VkBuffer vertexBuffer;
VkBuffer indexBuffer;

Engine::Engine(int w, int h, const char* title) {
    screenWidth = w;
    screenHeight = h;
    GLFWwindow* window = userWindow.initWindow(w, h, title);
    engInit.initialize(window);
    engGraphics.initialize(&engInit);

    create::QueueData indices(engInit.physicalDevice, engInit.surface);
    queueG = indices.graphicsFamily.value();
    queueP = indices.presentFamily.value();

    //creates vertex buffer
    createVertexBuffer(&gpuMemory);
    createIndexBuffer(&indexMemory);

    std::cout << "hi yah" << std::endl;
    //create uniform buffer
    //uniformMemory = createBuffer(&uniformBuffer, 5e7, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    //update uniform buffer memory
    //writeToBuffer(&engGraphics.ubo, sizeof(engGraphics.ubo), &uniformMemory);

    //engGraphics.createDescriptorSets(uniformBuffer);

    glfWindow = window;

}

Engine::~Engine() {
    mem::maDestroyMemory(engInit.device, gpuMemory);
    mem::maDestroyMemory(engInit.device, indexMemory);
    //mem::maDestroyMemory(engInit.device, uniformMemory, uniformBuffer);
    //nothing to do now, maybe in the future we can add a check to see if memory is leaking
    std::cout << "program exectuted and closed" << std::endl;
}

void Engine::draw() {

    engGraphics.drawFrame();
}

void Engine::destroyUniformData(size_t objIndex) {
    std::cout << "this should only run once" << std::endl;
    mem::maDestroyMemory(engInit.device, engGraphics.uniformBufferData[objIndex]);
}

/// - PURPOSE - 
/// take a given transform and apply it the object specified by the objIndex
/// - PARAMETERS - 
/// [glm::mat4] transform - the transform being applied to the object
/// [size_t] objIndex - the index representing the order at which the object was created.
/// - RETURNS - 
/// NONE
/// - NOTES - 
/// : need functionality to choose which object i am applying the transformation for
/// : a set system could possibly be a more efficient method than using vectors
void Engine::applyTransform(glm::mat4 transform, size_t objIndex, float camera_angle) {
    //add transform to buffer
    UniformBufferObject ubo;
    //ubo.modelToWorld = transform;
    //float lookAtVec = glm::dot(glm::vec3(0.229416,  0.688247, 0.688247), glm::vec3(-0.948683,  0.316228, 0));
    //printf("vec: <%f> \n", lookAtVec);

    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    ubo.modelToWorld = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    //ubo.worldToCamera = createCameraMatrix(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 3.0f, 3.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.worldToCamera = worldToCameraMatrix;
    //ubo.projection = glm::perspective(glm::radians(45.0f), engGraphics.swapChainExtent.width / (float) engGraphics.swapChainExtent.height, 0.1f, 10.0f); 
    
    ubo.projection = projectionMatrix;

    //printf("so this function should be called multiple times \n");

    //create buffers if needed
    if (objIndex >= engGraphics.uniformBufferData.size()) {
        //std::cout << objIndex << " oh yea!" << std::endl;
        engGraphics.uniformBufferData.resize(objIndex + 1);
        //need to create new buffer
        mem::MaMemory uniformBufferMemory;
        createUniformBuffer(sizeof(ubo), &uniformBufferMemory);
        engGraphics.uniformBufferData[objIndex] = uniformBufferMemory;
    }

    //finish?

    //the data is now written to memory in a uniform buffer
    //we just need a descriptor pool and set to describe the resource to the gpu
    if (objIndex + 1 > engGraphics.descriptorPools.size()) {
        engGraphics.createDescriptorPools();
        engGraphics.createDescriptorSets(sizeof(UniformBufferObject), engGraphics.uniformBufferData[objIndex].buffer);
    }

    writeToLocalBuffer(sizeof(UniformBufferObject), &engGraphics.uniformBufferData[objIndex], &ubo);
}

void Engine::sendCameraData(glm::mat4 worldToCamera, glm::mat4 projection) {
    cameraInit = true;

    worldToCameraMatrix = worldToCamera;
    projectionMatrix = projection;


}

//PURPOSE - create a buffer with the uniform bit active that is located in cpu-readable memory
//PARAMETERS - [VkDeviceSize] dataSize (the size of the data that will be allocated to this buffer)
//             [mem::MaMemory*] pMemory (pointer to the MaMemory obj that will contain the neccesary data on the buffer
//RETURNS - NONE
void Engine::createUniformBuffer(VkDeviceSize dataSize, mem::MaMemory* pMemory) {
    mem::MaBufferCreateInfo bufferInfo{};
    bufferInfo.size = dataSize;
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 1;
    bufferInfo.pQueueFamilyIndices = &queueG;
    bufferInfo.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    mem::maCreateBuffer(engInit.physicalDevice, engInit.device, &bufferInfo, pMemory);
}

//PURPOSE - write data to buffer in cpu-readable memory
//PARAMETERS - [VkDeviceSize] dataSize (size of the data being allocated)
//           - [mem::MaMemory*] pMemory (pointer to the memory where the data is being allocated)
//           - [void*] data (the information being allocated)
//RETURNS - NONE
void Engine::writeToLocalBuffer(VkDeviceSize dataSize, mem::MaMemory* pMemory, void* data) {
    mem::maAllocateMemory(dataSize, pMemory);
    mem::maMapMemory(engInit.device, dataSize, pMemory, data);
}

//PURPOSE - write data to buffer in device local memory, do NOT use this function for other memory types
//PARAMETERS - dataSize : (the size of the data being given to buffer)
//           - dstBuffer :  (the device local buffer the data will be written to)
//           - data : (the actual data that will be given to the buffer)/
//RETURNS - NONE
void Engine::writeToDeviceBuffer(VkDeviceSize dataSize, mem::MaMemory* pMemory, void* data) {
    //create and allocate memory for a temporary cpu readable buffer
    mem::MaMemoryData tempMemoryInfo{};
    mem::MaMemory tempMemory{};

    createTempBuffer(dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &tempMemory);
    mem::maMapMemory(engInit.device, dataSize, &tempMemory, data);

    //allocate memory in the device local buffer
    mem::maAllocateMemory(dataSize, pMemory);

    //transfer memory from temp buffer to given device local buffer
    engGraphics.copyBuffer(tempMemory.buffer, pMemory->buffer, pMemory->offset, dataSize);

    //make sure to state that the memory allocation is complete (maybe worth being implemented not by the user)
    pMemory->allocate = false;

    //destroy buffer and its associated memory
    mem::maDestroyMemory(engInit.device, tempMemory);

    std::cout <<  "hellow as up" << std::endl;
}

//PURPOSE - abstract a bit away from the underlying api when writing to the proper buffer
//PARAMETERS - [VkDeviceSize] dataSize - the size of the data being written to vertex buffer
//           - [void*] data - the data being written to vertex buffer
//RETURNS - NONE
void Engine::writeToVertexBuffer(VkDeviceSize dataSize, void* data) {
    writeToDeviceBuffer(dataSize, &gpuMemory, data);
}

void Engine::writeToIndexBuffer(VkDeviceSize dataSize, void* data) {
    writeToDeviceBuffer(dataSize, &indexMemory, data);
}

//PURPOSE - when function is called the render commands are updated so the neweset vertices are being drawn.
//PARAMETERS - [uint16_t] indexCount - a count of how many indices the the renderer has to draw
//RETURNS - NONE
void Engine::addToRenderQueue(uint16_t indexCount) {
    std::cout << "x" << std::endl;
    engGraphics.createCommandBuffers(gpuMemory.buffer, indexMemory.buffer);
}

//PURPOSE - create and allocate memory for a small cpu-readable buffer
//PARAMETERS  - [VkDeviceSize] dataSize (size of the data that will be allocated to this buffer)
//            - [VkBufferUsageFlags] usage (the type of buffer this buffer will transfer data to)
//            - [VkBuffer*] tempBuffer (memory location for the temp buffer to be created to)
//            - [mem::MaMemory*] tempMemory (memory location for the temp memory to be created to)
//RETURNS - [mem::MaMemoryData] memoryData (struct data giving information on where to allocate the data to)
void Engine::createTempBuffer(VkDeviceSize dataSize, VkBufferUsageFlags usage, mem::MaMemory* tempMemory) {
    mem::MaBufferCreateInfo bufferInfo{};
    bufferInfo.size = dataSize;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 1;
    bufferInfo.pQueueFamilyIndices = &queueG;
    bufferInfo.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    mem::maCreateBuffer(engInit.physicalDevice, engInit.device, &bufferInfo, tempMemory);

    //allocate memory for temp buffer
    mem::maAllocateMemory(dataSize, tempMemory);

    std::cout << "memory is allocated? : " << tempMemory->allocate << std::endl;
}

//TODO: put output variables as the last parameters
/*
mem::MaMemory Engine::createBuffer(VkBuffer* buffer, VkDeviceSize memorySize, VkBufferUsageFlags usage,
VkMemoryPropertyFlags properties) {
    create::QueueData indices(engInit.physicalDevice, engInit.surface);

    mem::MaMemoryInfo memoryInfo{};
    memoryInfo.allocationSize = memorySize;
    memoryInfo.bufferUsage = usage;
    memoryInfo.memoryProperties = properties;
    memoryInfo.queueFamilyIndexCount = indices.graphicsFamily.value();

    mem::MaMemory memoryObj{};

    mem::MaBufferCreateInfo bufferInfo{};
    bufferInfo.
    //mem::maCreateMemory(engInit.physicalDevice, engInit.device, &memoryInfo, buffer, &memoryObj);

    //return memoryObj;
}
*/
//PURPOSE - create a vertex buffer and assigned 5e7 bytes of device local memory
//PARAMETERS - [mem::MaMemory*] pMemory (a data struct containing information on the memory associated with the buffer)
//RETURNS - NONE
void Engine::createVertexBuffer(mem::MaMemory* pMemory) {
    //create::QueueData indices(engInit.physicalDevice, engInit.surface);

    mem::MaBufferCreateInfo bufferInfo{};
    bufferInfo.size = (VkDeviceSize) 5e7;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 1;
    bufferInfo.pQueueFamilyIndices = &queueG;
    bufferInfo.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    mem::maCreateBuffer(engInit.physicalDevice, engInit.device, &bufferInfo, pMemory);
}

//PURPOSE - create a index buffer and assigned 5e7 bytes of device local memory
//PARAMETERS - [mem::MaMemory*] pMemory (a data struct containing information on the memory associated with the buffer)
//RETURNS - NONE
void Engine::createIndexBuffer(mem::MaMemory* pMemory) {
    //create::QueueData indices(engInit.physicalDevice, engInit.surface);

    mem::MaBufferCreateInfo bufferInfo{};
    bufferInfo.size = (VkDeviceSize) 5e7;
    bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 1;
    bufferInfo.pQueueFamilyIndices = &queueG;
    bufferInfo.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    mem::maCreateBuffer(engInit.physicalDevice, engInit.device, &bufferInfo, pMemory);
}


float Engine::screenToVulkan(int screenCoord, int screenSize, int vulkanMin) {
    const float vulkanMaxWidth = 1;
    const float vulkanMinWidth = -1;

    float newRange = vulkanMaxWidth - vulkanMinWidth; //vulkan will convert both width and height to the same range (-1 , 1)
    float oldRange = (float) (screenSize - 0);
    float vulkanCoord = ((screenCoord * (newRange)) / oldRange) + vulkanMin;

    //std::cout << (screenCoord * newRange) << std::endl;

    return vulkanCoord;
}
