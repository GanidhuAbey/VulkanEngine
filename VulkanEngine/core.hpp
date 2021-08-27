#pragma once
#include "engine_init.hpp"
#include "eng_window.hpp"
#include "engine_graphics.hpp"
#include "memory_allocator.hpp"
#include "model.hpp"

#include "data_formats.hpp"

#define GLM_SWIZZLE
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

namespace core {

class Core {
    private:
        GLFWwindow* glfWindow;

        uint32_t primitiveCount = 0;
        
        glm::mat4 worldToCameraMatrix;
        glm::mat4 projectionMatrix;

    public:
        uint32_t queueG;
        uint32_t queueP;

        bool cameraInit = false;

        EngWindow userWindow;
        EngineInit engInit;
        graphics::EngineGraphics engGraphics;

        int screenWidth;
        int screenHeight;

        mem::MaMemory gpuMemory;
        mem::MaMemory indexMemory;

        mem::MaMemoryData memoryData;
        //draw::EngineDraw engineDraw;
        //in the future we can make this an array of vertices to hold multiple gameobject data.


    public:
        void init(int w, int h, const char* title);
        ~Core();

    public:
        //refer to brendan galea vid on how to properly seperate gameobjects
        void draw();
        float screenToVulkan(int screenCoord, int screenSize, int vulkanMin);
        void writeToVertexBuffer(std::vector<data::Vertex> vertices);
        void writeToIndexBuffer(std::vector<uint32_t> indices);
        void writeToNormalBuffer(VkDeviceSize dataSize, void* data);
        void applyTransform(glm::mat4 transform, size_t objIndex, float camera_angle);
        void destroyUniformData(size_t objIndex);

        void sendCameraData(glm::mat4 worldToCamera, glm::mat4 projection);

        bool hasUniformBuffer(size_t objIndex);
        void attachData(UniformBufferObject ubo);
        void updateData(UniformBufferObject ubo, size_t objIndex);
        void createCommands(std::vector<model::Model*> allModels, LightObject light, std::vector<PushFragConstant> pfcs);
        void updateBuffers(model::Model* model);
        void attachTextureData(model::Model* modelData, size_t index);

    private:
        void createVertexBuffer(mem::MaMemory* pMemory);
        void createNormalBuffer(mem::MaMemory* pMemory);
        void createIndexBuffer(mem::MaMemory* pMemory);
        void writeToDeviceBuffer(VkDeviceSize dataSize, mem::MaMemory* pMemory, void* data);
        void createTempBuffer(VkDeviceSize dataSize, VkBufferUsageFlags usage, mem::MaMemory* tempMemory);
        void createUniformBuffer(VkDeviceSize dataSize, mem::MaMemory* pMemory);
        void writeToLocalBuffer(VkDeviceSize dataSize, mem::MaMemory* pMemory, void* data);



};
}
