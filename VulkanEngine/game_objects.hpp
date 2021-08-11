#include "engine.hpp"

#include <string.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace gameObject {
//empty object
class EmptyObject {
    private:
        create::Engine* engine;
    public:
        glm::mat4 objTransform; //matrix representing the linear transformation the user wants to apply (translate, rotate, scale)
    public:
        EmptyObject(create::Engine* eng);
        ~EmptyObject();
        void addMeshData(const std::string& fileName);
        void addTransform(uint32_t x, uint32_t y, uint32_t z, float camera_angle);
    private:
        //void abstractToVector(std::vector* vector, void* data);
};

//square object
class Square {
    private:
        create::Engine* engine;
    public:
        Square(int x, int y, int s, Color c, create::Engine* eng);
        ~Square();
        void translate(int x, int y);
        void rotate(float angle);
        void material(const char texture);
};

//cube object
class Cube {
    private:
        create::Engine* engine;
    public:
        //for now the z coordinate won't do anything
        Cube(int x, int y, int z, int s, Color c, create::Engine* eng);
        ~Cube();
        void rotate(float angle);
};

//light object
class Light {
    private:
        create::Engine* engine;

    public:
        Light(int x, int y, int z, Color c, create::Engine* eng);
        ~Light();

};

}

