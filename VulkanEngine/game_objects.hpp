#include "core.hpp"

#include <string.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace gameObject {
//empty object
std::tuple<std::vector<data::Vertex>, std::vector<uint16_t>> readObjData(const std::string& fileName);
}

