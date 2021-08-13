#include "camera.hpp"

using namespace camera;



/// - PURPOSE - 
/// construct a matrix that transforms objects relative to the camera
// - PARAMETERS - 
/// [glm::vec3] lookAt - vector representing the point the camera is looking at
/// [glm::vec3] cameraPos - vector representing the point the camera is located at
// - RETURNS - 
/// [glm::mat4] worldToCamera - matrix representing the transformation required to get to camera space
/// - NOTES - 
/// for some reason matrix multiplication was not giving me the expected output
glm::mat4 camera::createCameraMatrix(glm::vec3 cameraPos, glm::vec3 lookAt, glm::vec3 up) {
    //we know we're looking at the point described by the lookAt matrix, but where is the camera?
    //this vector should also represent the distance we need to translate the object by;
    glm::vec3 camera_z = glm::normalize(lookAt - cameraPos);
    glm::vec3 camera_x = glm::normalize(glm::cross(camera_z, up)); //the axis dont have to be perpendicular to use the cross product

    //so  if i use camera_x and camera_z i can calculate a camera y that is perpendicular to the other two vectors
    glm::vec3 camera_y = glm::cross(camera_x, camera_z);

    camera_z = -camera_z;

    glm::mat4 modelToWorld = {
        camera_x.x, camera_x.y, camera_x.z, -glm::dot(camera_x, cameraPos),
        camera_y.x, camera_y.y, camera_y.z, -glm::dot(camera_y, cameraPos),
        camera_z.x, camera_z.y, camera_z.z, -glm::dot(camera_z, cameraPos),
        0, 0, 0, 1,
    };

    return glm::transpose(modelToWorld);
}

glm::mat4 camera::createOrthogonalMatrix(int width, int height, float n, float f) {
    float r = (float) width/2;
    float t = (float) height/2;
    //the given parameters define a cube the governs the space that our input values live in. we must then convert this cube
    //into a normalized cube with a center located at (0, 0, 0). finally we then project this object onto the screen orthogonaly

    //translate the cube
    glm::mat4 translate = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, -(f+n)/2,
        0, 0, 0, 1,
    };

    //normalize the cube 
    glm::mat4 normalize = {
        1/(r), 0, 0, 0,
        0, 1/(t), 0, 0,
        0, 0, 2/(f-n), 0,
        0, 0, 0, 1,
    };

    //project the cube space onto a plane
    glm::mat4 project = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, -1, 0,
        0, 0, 0, 1,
    };

    //multiply these vectors and return the output
    return glm::transpose(project * normalize * translate);
}

glm::mat4 camera::createPerspectiveMatrix(float angle, float aspect, float n, float f) {
    //find the distance to the "screen" that will be mapped to
    //the distance to the screen is related to the angle between 'l' and 'r' by the eqn : phi = 2arctan(w/2d)
    //d = w/(2*tan(phi/2)) <-- how can i derive the the neccesarry information to solve this from the parameters used by glm::perspective?
    double c = 1.0/(glm::tan(angle/2));

    //translate and normalize the vertices to bring it the unit cube form
    glm::mat4 projection_simplified = {
        c/aspect, 0, 0, 0,
        0, c, 0, 0,
        0, 0, -(f+n)/(f-n), -(2*f*n)/(f-n),
        0, 0, -1, 0,
    };

    return glm::transpose(projection_simplified);
    //project the points to a plane
}