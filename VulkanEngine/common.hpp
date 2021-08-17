#pragma once

#include "data_formats.hpp"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <vector>
#include <fstream>

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

struct LightObject {
    glm::vec4 color;
    glm::vec4 position;
};

struct PushFragConstant {
    glm::vec4 color;
};

static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("could not open file");
    }

    size_t fileSize = (size_t)file.tellg();

    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

