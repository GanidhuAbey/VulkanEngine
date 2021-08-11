#pragma once

#include "data_formats.hpp"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <vector>

extern uint16_t indexCount;
extern uint16_t vertexCount;
extern uint16_t objectCount;

extern VkBuffer vertexBuffer;
extern VkBuffer indexBuffer;


//make a wrapper for the GLFW inputs so the user doesnt have to interact with multiple different libraries
enum WindowInput {
    W = GLFW_KEY_W,
    A = GLFW_KEY_A,
    S = GLFW_KEY_S,
    D = GLFW_KEY_D,
    X = GLFW_KEY_X,
};

struct UniformBufferObject {
    glm::mat4 modelToWorld;
    glm::mat4 worldToCamera;
    glm::mat4 projection;
};
