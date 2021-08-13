#include "core.hpp"

#include <glm/glm.hpp>

namespace camera {

glm::mat4 createCameraMatrix(glm::vec3 cameraPos, glm::vec3 lookAt, glm::vec3 up);
glm::mat4 createOrthogonalMatrix(int width, int height, float n, float f);
glm::mat4 createPerspectiveMatrix(float angle, float aspect, float n, float f);

}