#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "swapchain_support.hpp"
#include "queue.hpp"
#include "data_formats.hpp"
#include "memory_allocator.hpp"
#include "mesh.hpp"

#include "engine_init.hpp"

#include <cstdlib>
#include <algorithm>
#include <chrono>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

const int MAX_FRAMES_IN_FLIGHT = 3;

namespace graphics {

class EngineGraphics {
    public:
        VkPipeline graphicsPipeline;
        uint32_t bindingCount = 1;
        uint32_t bufferSize = 0;
        std::vector<data::Vertex> oldVertices;
        std::vector<VkCommandBuffer> commandBuffers;

        std::vector<VkDescriptorPool> descriptorPools;
        std::vector<std::vector<VkDescriptorSet>> descriptorSets;

        std::vector<mem::MaMemory> uniformBufferData;

        VkExtent2D swapChainExtent;

    private:
        VkFormat swapChainImageFormat;

        VkRenderPass renderPass;

        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainColorImageViews;

        VkImage depthImage;
        VkImageView depthImageView;
        VkDeviceMemory depthMemory;

        //std::vector<VkImageView> swapChainDepthImageViews;

        VkSwapchainKHR swapChain;

        core::EngineInit* engineInit;

        uint32_t attributeCount = 2;

        VkDescriptorSetLayout setLayout;
        VkPipelineLayout pipelineLayout;

        std::vector<VkFramebuffer> swapChainFramebuffers;

        //this one signals when an image has been presented and is available for rendering
        std::vector<VkSemaphore> imageAvailableSemaphores;
        //this one signals when an image has finished been rendered to and is awaiting presentation
        std::vector<VkSemaphore> renderFinishedSemaphores;
        //this fence signals when a frame has run through and returned from being rendered and presented
        //so that the current frame can be drawn again.
        //this prevents the cpu from idly forcing draw calls while images are not available.
        std::vector<VkFence> inFlightFences;
        size_t currentFrame = 0;
        //adds another check that the image we are about to send to flight, is not an image that was rendered in
        //the last frame (if it was then we wait the fence to return a signal before we continue) and then we dont waste
        //energy
        std::vector<VkFence> imagesInFlight;

        std::vector<mesh::Mesh> recentMeshData;
        LightObject recentLightObject;
        std::vector<PushFragConstant> recentPushConstants;

    private:
        void recreateSwapChain();
        void cleanupSwapChain(bool destroyAll);
        VkPipelineShaderStageCreateInfo fillShaderStageStruct(VkShaderStageFlagBits stage, VkShaderModule shaderModule);
        VkShaderModule createShaderModule(std::vector<char> shaderCode);
        VkSurfaceFormatKHR chooseSwapChainFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapChainPresentation(const std::vector<VkPresentModeKHR>& availablePresentations);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
        void createImage(VkFormat format, VkImageUsageFlags usage, VkImage* images);
    	void createDepthImage();
        void createImageMemory(VkImage image);
        void createImageView(VkFormat format, VkImageUsageFlags usage, VkImage image, VkImageAspectFlags aspectFlags, VkImageView* imageView);
        void createCommandBuffer(VkCommandBuffer commandBuffer, VkFramebuffer framebuffer, std::vector<VkDescriptorSet> descriptorSet, VkBuffer vertexBuffer, VkBuffer indexBuffer, 
            std::vector<mesh::Mesh> allMeshData, LightObject light, std::vector<PushFragConstant> pfcs);

    public:
        //EngineGraphics(EngineInit* initEngine);
        void initialize(core::EngineInit* initEngine);
        ~EngineGraphics();

    private:
        void createSwapChain(); //
        void createDepthResources();
        void createColorImageViews();  //
        void createRenderPass(); //
        void createDescriptorSetLayout();
        void createGraphicsPipeline(); //
        void createFrameBuffers(); //
        void createSemaphores();
        void createFences();

    public:
        void createCommandBuffers(VkBuffer buffer, VkBuffer indBuffer,  std::vector<mesh::Mesh> allMeshData, LightObject light, std::vector<PushFragConstant> pfcs);
        void drawFrame();
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size);
        void updateDescriptorSet(VkDescriptorSet decriptorSet, VkDeviceSize bufferSize, VkBuffer buffer);
        void createDescriptorPools();
        void createDescriptorSets(VkDeviceSize dataSize, VkBuffer buffer);
};

}
