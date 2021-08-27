//this core file acts as the 'link' between the backend functions

#include "core.hpp"
#include "common.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace core;

//define some global variables.
uint32_t objectCount = 0;

VkBuffer vertexBuffer;
VkBuffer indexBuffer;

void Core::init(int w, int h, const char* title) {
    screenWidth = w;
    screenHeight = h;
    GLFWwindow* window = userWindow.initWindow(w, h, title);
    engInit.initialize(window);
    engGraphics.initialize(&engInit);

    core::QueueData indices(engInit.physicalDevice, engInit.surface);
    queueG = indices.graphicsFamily.value();
    queueP = indices.presentFamily.value();

    //creates vertex buffer
    createVertexBuffer(&gpuMemory);
    createIndexBuffer(&indexMemory);

    //create uniform buffer
    //uniformMemory = createBuffer(&uniformBuffer, 5e7, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    //update uniform buffer memory
    //writeToBuffer(&engGraphics.ubo, sizeof(engGraphics.ubo), &uniformMemory);

    //engGraphics.createDescriptorSets(uniformBuffer);

    glfWindow = window;

}

Core::~Core() {
    mem::destroyBuffer(engInit.device, gpuMemory);
    mem::destroyBuffer(engInit.device, indexMemory);
    //mem::destroyBuffer(engInit.device, uniformMemory, uniformBuffer);
    //nothing to do now, maybe in the future we can add a check to see if memory is leaking
    std::cout << "program exectuted and closed" << std::endl;
}

void Core::draw() {
    engGraphics.drawFrame();
}

void Core::destroyUniformData(size_t objIndex) {
    mem::destroyBuffer(engInit.device, engGraphics.uniformBufferData[objIndex]);
}

bool Core::hasUniformBuffer(size_t objIndex) {
    return ((objIndex + 1) <= engGraphics.uniformBufferData.size());
}

void Core::attachData(UniformBufferObject ubo) {
    size_t objIndex = engGraphics.uniformBufferData.size();
    engGraphics.uniformBufferData.resize(objIndex + 1);
    //need to create new buffer
    mem::MaMemory uniformBufferMemory;
    createUniformBuffer(sizeof(ubo), &uniformBufferMemory);
    engGraphics.uniformBufferData[objIndex] = uniformBufferMemory;

    engGraphics.createUBOPools();
    printf("hello \n");
    engGraphics.createUBOSets(sizeof(UniformBufferObject), engGraphics.uniformBufferData[objIndex].buffer);
}

void Core::attachTextureData(model::Model* modelData, size_t index) {
    //generate images here
    for (size_t i = 0; i < modelData->modelMeshes.size(); i++) {
        printf("THE MODEL MESH SIZE IF : %zu", modelData->modelMeshes.size());
        std::vector<aiString> meshTextures = modelData->modelMeshes[i].getTexturePaths();

        //load image with path, will need different library
        //TODO: when multiple textures are present a blending of some sort may be required before being sent to the shader
        if (meshTextures.size() == 0) {
            printf("why no pass throug here \n");
            return;
        }

        //printf("the image path is: %s \n", meshTextures[0]);
        int imageWidth, imageHeight, imageChannels;
        stbi_uc *pixels = stbi_load(meshTextures[0].data, &imageWidth, &imageHeight, &imageChannels, STBI_rgb_alpha);

        VkDeviceSize dataSize = 4 * (imageWidth * imageHeight);
        mem::MaBufferCreateInfo textureBufferInfo{};
        textureBufferInfo.size = dataSize;
        textureBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        textureBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        textureBufferInfo.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        textureBufferInfo.queueFamilyIndexCount = 1;
        textureBufferInfo.pQueueFamilyIndices = &queueG;

        printf("data size: %zu \n", dataSize);

        mem::MaMemory newBuffer;
        mem::maCreateBuffer(engInit.physicalDevice, engInit.device, &textureBufferInfo, &newBuffer);
        mem::maAllocateMemory(dataSize, &newBuffer);
        mem::maMapMemory(engInit.device, dataSize, &newBuffer, pixels);

        printf("made it ? \n");

        //use size of loaded image to create VkImage
        mem::MaMemory newTextureImage;

        mem::MaImageCreateInfo imageInfo{};
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        VkExtent3D extent{};
        extent.width = static_cast<uint32_t>(imageWidth);
        extent.height = static_cast<uint32_t>(imageHeight);
        extent.depth = 1; //this depth might be key to blending the textures...
        imageInfo.extent = extent;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.queueFamilyIndexCount = 1;
        imageInfo.pQueueFamilyIndices = &queueG;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        mem::maCreateImage(engInit.physicalDevice, engInit.device, &imageInfo, &newTextureImage);

        //mem::maAllocateMemory(dataSize, &newTextureImage);
        //transfer the image to appropriate layout for copying
        engGraphics.transferImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &newTextureImage.image);
        engGraphics.copyImage(newBuffer.buffer, newTextureImage.image, 0.0, imageWidth, imageHeight);
        //transfer the image again to a more optimal layout for texture sampling?
        engGraphics.transferImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &newTextureImage.image);

        //create image view for image
        mem::ImageViewCreateInfo viewInfo{};
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        mem::createImageView(engInit.device, viewInfo, &newTextureImage);

        mem::destroyBuffer(engInit.device, newBuffer);

        //save texture image to mesh
        modelData->modelMeshes[i].addTexture(newTextureImage);

        //save image data to texture
        modelData->modelMeshes[i].saveDimensions(static_cast<uint32_t>(imageWidth), static_cast<uint32_t>(imageHeight));
     
        //free image
        stbi_image_free(pixels);
    }

    /* Create Descriptor Data For Model */
    engGraphics.createTextureDescriptorData(modelData, index);
}

void Core::updateData(UniformBufferObject ubo, size_t objIndex) {
    //need to map new data into this buffer
    //free memory
    mem::MaFreeMemoryInfo freeInfo{};
    freeInfo.deleteOffset = 0;
    freeInfo.deleteSize = sizeof(ubo);
    mem::maFreeMemory(freeInfo, &engGraphics.uniformBufferData[objIndex]);
    writeToLocalBuffer(sizeof(ubo), &engGraphics.uniformBufferData[objIndex], &ubo);
}

void Core::createCommands(std::vector<model::Model*> allModels, LightObject light, std::vector<PushFragConstant> pfcs) {
    //needs to create command buffers
    printf("the size of all models is : %u \n", allModels.size());
    engGraphics.createCommandBuffers(gpuMemory.buffer, indexMemory.buffer, allModels, light, pfcs);
}


void Core::updateBuffers(model::Model* model) {
    printf("right? \n");
    std::vector<mesh::Mesh> modelMeshes = model->modelMeshes;
    printf("going in \n");
    for (auto& currentMesh : modelMeshes) {
        std::vector<data::Vertex> verts = currentMesh.getVertexData();
        std::vector<uint32_t> indexes = currentMesh.getIndexData();

        printf("size of verts: %u | size of indices: %u", verts.size(), indexes.size());

        writeToVertexBuffer(verts);
        writeToIndexBuffer(indexes);
    }
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
void Core::applyTransform(glm::mat4 transform, size_t objIndex, float camera_angle) {
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
        engGraphics.createUBOPools();
        engGraphics.createUBOSets(sizeof(UniformBufferObject), engGraphics.uniformBufferData[objIndex].buffer);
    }

    writeToLocalBuffer(sizeof(UniformBufferObject), &engGraphics.uniformBufferData[objIndex], &ubo);
}

void Core::sendCameraData(glm::mat4 worldToCamera, glm::mat4 projection) {
    cameraInit = true;

    worldToCameraMatrix = worldToCamera;
    projectionMatrix = projection;


}

//PURPOSE - create a buffer with the uniform bit active that is located in cpu-readable memory
//PARAMETERS - [VkDeviceSize] dataSize (the size of the data that will be allocated to this buffer)
//             [mem::MaMemory*] pMemory (pointer to the MaMemory obj that will contain the neccesary data on the buffer
//RETURNS - NONE
void Core::createUniformBuffer(VkDeviceSize dataSize, mem::MaMemory* pMemory) {
    mem::MaBufferCreateInfo bufferInfo{};
    bufferInfo.size = dataSize;
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 1;
    bufferInfo.pQueueFamilyIndices = &queueG;
    bufferInfo.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    printf("creating uniform buffer \n");

    mem::maCreateBuffer(engInit.physicalDevice, engInit.device, &bufferInfo, pMemory);
}

//PURPOSE - write data to buffer in cpu-readable memory
//PARAMETERS - [VkDeviceSize] dataSize (size of the data being allocated)
//           - [mem::MaMemory*] pMemory (pointer to the memory where the data is being allocated)
//           - [void*] data (the information being allocated)
//RETURNS - NONE
void Core::writeToLocalBuffer(VkDeviceSize dataSize, mem::MaMemory* pMemory, void* data) {
    mem::maAllocateMemory(dataSize, pMemory);
    mem::maMapMemory(engInit.device, dataSize, pMemory, data);
}

//PURPOSE - write data to buffer in device local memory, do NOT use this function for other memory types
//PARAMETERS - dataSize : (the size of the data being given to buffer)
//           - dstBuffer :  (the device local buffer the data will be written to)
//           - data : (the actual data that will be given to the buffer)/
//RETURNS - NONE
void Core::writeToDeviceBuffer(VkDeviceSize dataSize, mem::MaMemory* pMemory, void* data) {
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
    mem::destroyBuffer(engInit.device, tempMemory);
}

//PURPOSE - abstract a bit away from the underlying api when writing to the proper buffer
//PARAMETERS - [VkDeviceSize] dataSize - the size of the data being written to vertex buffer
//           - [void*] data - the data being written to vertex buffer
//RETURNS - NONE
void Core::writeToVertexBuffer(std::vector<data::Vertex> vertices) {
    writeToDeviceBuffer(sizeof(vertices[0]) * vertices.size(), &gpuMemory, vertices.data());
}

void Core::writeToIndexBuffer(std::vector<uint32_t> indices) {
    writeToDeviceBuffer(sizeof(indices[0]) * indices.size(), &indexMemory, indices.data());
}

//PURPOSE - create and allocate memory for a small cpu-readable buffer
//PARAMETERS  - [VkDeviceSize] dataSize (size of the data that will be allocated to this buffer)
//            - [VkBufferUsageFlags] usage (the type of buffer this buffer will transfer data to)
//            - [VkBuffer*] tempBuffer (memory location for the temp buffer to be created to)
//            - [mem::MaMemory*] tempMemory (memory location for the temp memory to be created to)
//RETURNS - [mem::MaMemoryData] memoryData (struct data giving information on where to allocate the data to)
void Core::createTempBuffer(VkDeviceSize dataSize, VkBufferUsageFlags usage, mem::MaMemory* tempMemory) {
    mem::MaBufferCreateInfo bufferInfo{};
    bufferInfo.size = dataSize;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 1;
    bufferInfo.pQueueFamilyIndices = &queueG;
    bufferInfo.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    printf("creating temp buffer \n");

    mem::maCreateBuffer(engInit.physicalDevice, engInit.device, &bufferInfo, tempMemory);

    //allocate memory for temp buffer
    mem::maAllocateMemory(dataSize, tempMemory);

}

//TODO: put output variables as the last parameters
/*
mem::MaMemory Core::createBuffer(VkBuffer* buffer, VkDeviceSize memorySize, VkBufferUsageFlags usage,
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
void Core::createVertexBuffer(mem::MaMemory* pMemory) {
    //create::QueueData indices(engInit.physicalDevice, engInit.surface);

    mem::MaBufferCreateInfo bufferInfo{};
    bufferInfo.size = (VkDeviceSize) 5e7;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 1;
    bufferInfo.pQueueFamilyIndices = &queueG;
    bufferInfo.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    printf("creating vertex buffer \n");

    mem::maCreateBuffer(engInit.physicalDevice, engInit.device, &bufferInfo, pMemory);
}

//PURPOSE - create a index buffer and assigned 5e7 bytes of device local memory
//PARAMETERS - [mem::MaMemory*] pMemory (a data struct containing information on the memory associated with the buffer)
//RETURNS - NONE
void Core::createIndexBuffer(mem::MaMemory* pMemory) {
    //create::QueueData indices(engInit.physicalDevice, engInit.surface);

    mem::MaBufferCreateInfo bufferInfo{};
    bufferInfo.size = (VkDeviceSize) 5e7;
    bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 1;
    bufferInfo.pQueueFamilyIndices = &queueG;
    bufferInfo.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    printf("creating index buffer \n");

    mem::maCreateBuffer(engInit.physicalDevice, engInit.device, &bufferInfo, pMemory);
}


float Core::screenToVulkan(int screenCoord, int screenSize, int vulkanMin) {
    const float vulkanMaxWidth = 1;
    const float vulkanMinWidth = -1;

    float newRange = vulkanMaxWidth - vulkanMinWidth; //vulkan will convert both width and height to the same range (-1 , 1)
    float oldRange = (float) (screenSize - 0);
    float vulkanCoord = ((screenCoord * (newRange)) / oldRange) + vulkanMin;


    return vulkanCoord;
}
