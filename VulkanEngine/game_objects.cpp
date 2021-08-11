#include "game_objects.hpp"
#include "common.hpp"
#include "math.h"
#include <string>

using namespace gameObject;


const int DEPTH_CONSTANT = 100;

//Empty GameObject --------------------------------------------------------------------------------------------
EmptyObject::EmptyObject(create::Engine* eng) {
    engine = eng;

    if (!engine->cameraInit) {
        throw std::runtime_error("a camera has not been placed into the scene!, you will not be able to render anything without it");
    }

    //add a default transform
    objTransform = glm::mat4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );
}

EmptyObject::~EmptyObject() {
    engine->destroyUniformData(0);
}

//PURPOSE - this function will read an obj file and parse its data into formats that our renderer can handle
//PARAMETERS - fileName (name of the obj file that data will be extracted from)
//NOTES - currently pretty slow for just a data parser, will leave as is for now and return when the bottleneck becomes more problematic
//      - if we had read the file more efficiently than we could save time here
void EmptyObject::addMeshData(const std::string& fileName) {
    //convert mesh data into vertices and index data.
    std::vector<char> meshData = engine->engInit.readFile(fileName);

    //iterate through the meshData file
    bool ignore_line = false;
    bool found = false;
    bool number = false;
    char desired = 'x';

    std::string value = "";
    std::string name = "";

    std::vector<char> vert_data;
    std::vector<char> index_data;

    std::vector<float> v;
    std::vector<float> vt;
    std::vector<float> vn;

    //when vt and vn are needed we can just add more variables here
    int vCount = 0;
    int vLineCount = 0;

    std::vector<uint16_t> indices;

    for (size_t i = 0; i < meshData.size(); i++) {
        char data = meshData[i];
        if (found && data != ' ' && data != '\n') {
            value += data;
            //std::cout << "herllo" << std::endl;
        }
        else {
            number = true;
        }

        //generated a value now we determine where to add it
        if (name == "v" && number && found) {
            //std::cout << "value: "  << value << "|" << std::endl;
            vCount++;
            v.push_back(std::stof(value));
            value = "";
        }

        if (name == "f" && number && found) {
            //std::cout << value << std::endl;
            std::string delimiter = "/";
            size_t count = 0;
            size_t pos = 0;
            std::vector<uint16_t> index_types(3); // im gonna regret putting a constant here ...
            while ((pos = value.find(delimiter)) != std::string::npos) {
                std::string token = value.substr(0, pos);
                index_types[count] = static_cast<uint16_t>( std::stoul(token) );
                count++;
                value.erase(0, pos + delimiter.length());
            }
            indices.push_back(index_types[0] - 1);
            value = "";
        }

        if (data != ' ' && !found) {
            name += data;
        }
        else {
            found = true;
            number = false;
            //value = "";
        }

        if (meshData[i] == '\n') {
            if (name == "v") {
                vLineCount++;
            }

            name = "";
            value = "";
            number = false;
            found = false;
        }
    }

    size_t vDataCount = vCount / vLineCount;

    std::cout << v.size() / vDataCount << std::endl;

    //last step is to convert them to our format
    std::vector<data::Vertex2D> vertices;
    for (size_t i = 0; i < v.size() / vDataCount; i++) {
        //for now i only accept sizes of 3
        if (vDataCount != 3) {
            std::cout << "WARNING: obj file vector-" << vDataCount << " but vector-3 expected for v" << std::endl;
        }

        //now we use this to create the vec4 we need
        size_t index = 3*i;
        glm::vec4 vertice = glm::vec4(v[index], v[index+1], v[index+2], 0.0);

        //materials haven't been implemented so we'll just do a default value here
        glm::vec3 color = glm::vec3(0.1, 0.2, 0.6);

        data::Vertex2D vertex = {
            vertice,
            color
        };

        vertices.push_back(vertex);
    }

    //print out vertices to check
    /*
    for (size_t i = 0; i < vertices.size(); i++) {
        //print position
        std::cout << vertices[i].position[0] << ", " << vertices[i].position[1] << ", " << vertices[i].position[2] << '\n';
        std::cout << vertices[i].color[0] << ", " << vertices[i].color[1] << ", " << vertices[i].color[2] << '\n';
    }
    */
    indexCount += (uint16_t) indices.size();

    engine->writeToVertexBuffer(sizeof(vertices[0]) * vertices.size(), vertices.data());
    engine->writeToIndexBuffer(sizeof(indices[0]) * indices.size(), indices.data());

    //not sure if the best place to put this...
    //not the place at ALL to put this functionality
    engine->applyTransform(objTransform, 0, 0);
    engine->addToRenderQueue(indexCount);
}

//PURPOSE - define the transformation of the object (rotation, scale, translation)
//PARAMTERS - [uint32_t] x - x translation
//          - [uint32_t] y - y translation
//          - [uint32_t] z - z translation
void EmptyObject::addTransform(uint32_t x, uint32_t y, uint32_t z, float camera_angle) {
    glm::mat4 objRotate = {
        cos(camera_angle), 0, sin(camera_angle), 0,
        0, 1, 0, 0,
        -sin(camera_angle), 0, cos(camera_angle), 0,
        0, 0, 0, 1,
    };

    glm::mat4 objTranslate = {
        1, 0, 0, x,
        0, 1, 0, y,
        0, 0, 1, z,
        0, 0, 0, 1,
    };

    objTransform = objTranslate * objRotate;

    //std::cout << "CAMERA " << camera_angle << std::endl;
    engine->applyTransform(objTransform, 0, camera_angle);
}

//Square GameObject  ------------------------------------------------------------------------------------------
Square::Square(int x, int y, int s, Color c, create::Engine* eng) {
    engine = eng;

    float vulkanX = engine->screenToVulkan(x, engine->screenWidth, -1);
    float vulkanY = engine->screenToVulkan(y, engine->screenHeight, -1);

    float vulkanWidth = engine->screenToVulkan(s, engine->screenWidth, 0);
    float vulkanHeight = engine->screenToVulkan(s, engine->screenHeight, 0);

    //create appropriate vertices
    //TODO: sort out a method to combine vertices (resize vertices array from current length to add new vertices),
    //      then specify a offset so command buffer can draw all vertices properly.
    std::vector<data::Vertex2D> vertices = {
        {{vulkanX - vulkanWidth, vulkanY - vulkanHeight, 0.0, 0.0}, {c.red, c.green, c.blue}},
        {{vulkanX + vulkanWidth, vulkanY - vulkanHeight, 0.0, 0.0}, {c.red, c.green, c.blue}},
        {{vulkanX + vulkanWidth, vulkanY + vulkanHeight, 0.0, 0.0}, {c.red, c.green, c.blue}},
        {{vulkanX - vulkanWidth, vulkanY + vulkanHeight, 0.0, 0.0}, {c.red, c.green, c.blue}}
    };

    std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0
    };

    indexCount += (uint16_t) indices.size();
    objectCount++;

    /*
    for (int i = 0; i < indices.size(); ++i) {
        std::cout << indices[i] << std::endl;
    }
    */

    /*
    for (size_t i = 0; i < engine->vertices.size(); ++i) {
        for (size_t j = 0; j < 2; ++j) {
            std::cout << engine->vertices[i].position[j] << ", ";
        }
        std::cout << "\n";
    }
    */

    /*
    engine->engGraphics.ubo.translation = glm::mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 0.0
    );
    */

    //upload the vertices data over here instead
    engine->writeToVertexBuffer(sizeof(vertices[0]) * vertices.size(), vertices.data());
    engine->writeToIndexBuffer(sizeof(indices[0]) * indices.size(), indices.data());

    //not sure if the best place to put this...
    //engine->engGraphics->createCommandBuffers(*vertexBuffer, indexBuffer, vertices, indices);
    engine->addToRenderQueue(indexCount);

}

Square::~Square() {
    //delete verticesz
}

void Square::translate(int x, int y) {
    //update the uniform buffer here
    float translateX = engine->screenToVulkan(x, engine->screenWidth, 0);
    float translateY = engine->screenToVulkan(y, engine->screenHeight, 0);

    /*
    engine->engGraphics.ubo.translation = glm::mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        translateX, translateY, 1.0, 0.0,
        0.0, 0.0, 0.0, 0.0
    );
    */

    //engine->engGraphics.updateUniformBuffers()
}

void Square::rotate(float angle) {
    float x = -sin(angle);
}

//-------------------------------------------------------------------------------------------------------------
//Cube GameObject  ------------------------------------------------------------------------------------------
Cube::Cube(int x, int y, int z, int s, Color c, create::Engine* eng) {
    engine = eng;

    float vulkanX = engine->screenToVulkan(x, engine->screenWidth, -1);
    float vulkanY = engine->screenToVulkan(y, engine->screenWidth, -1);

    float vulkanWidth = engine->screenToVulkan(s, engine->screenWidth, 0) / 2;
    float vulkanHeight = engine->screenToVulkan(s, engine->screenWidth, 0) / 2;

    //float vulkanWidth = s / DEPTH_CONSTANT;
    //float vulkanHeight = s / DEPTH_CONSTANT;

    //std::cout << vulkanWidth << std::endl;
    //std::cout << vulkanHeight << std::endl;
    //std::cout << s << std::endl;
    //std::cout << std::endl;

    //
    std::vector<data::Vertex2D> vertices = {
        {{vulkanX - vulkanWidth, vulkanY - vulkanHeight, z, 0.0}, {c.red, c.green, c.blue}},
        {{vulkanX + vulkanWidth, vulkanY - vulkanHeight, z, 0.0}, {c.red, c.green, c.blue}},
        {{vulkanX + vulkanWidth, vulkanY + vulkanHeight, z, 0.0}, {c.red, c.green, c.blue}},
        {{vulkanX - vulkanWidth, vulkanY + vulkanHeight, z, 0.0}, {c.red, c.green, c.blue}},
        {{vulkanX - vulkanWidth, vulkanY - vulkanHeight, vulkanWidth * 2 + z, 0.0}, {c.red, c.green, c.blue}},
        {{vulkanX + vulkanWidth, vulkanY - vulkanHeight, vulkanWidth * 2 + z, 0.0}, {c.red, c.green, c.blue}},
        {{vulkanX + vulkanWidth, vulkanY + vulkanHeight, vulkanWidth * 2 + z, 0.0}, {c.red, c.green, c.blue}},
        {{vulkanX - vulkanWidth, vulkanY + vulkanHeight, vulkanWidth * 2 + z, 0.0}, {c.red, c.green, c.blue}}
    };

    //the indices will get add on to the end to m
    std::vector<uint16_t> indices =  {
        2, 3, 0, 0, 1, 2,
        6, 5, 4, 4, 7, 6,
        3, 7, 4, 4, 0, 3,
        7, 3, 2, 2, 6, 7,
        5, 6, 2, 2, 1, 5,
        4, 5, 1, 1, 0, 4
    };

    for (size_t i = 0; i < indices.size(); i++) {
        indices[i] += vertexCount;
    }

    indexCount += (uint16_t) indices.size();
    objectCount++;

    //
    engine->writeToVertexBuffer(sizeof(vertices[0]) * vertices.size(), vertices.data());
    engine->writeToIndexBuffer(sizeof(indices[0]) * indices.size(), indices.data());

    engine->addToRenderQueue(indexCount);
}

Cube::~Cube() {}

void Cube::rotate(float angle) {
    /*
    engine->engGraphics.ubo.model = glm::mat4(
        cos(angle), 0, sin(angle), 0,
        0,          1,          0, 0,
        -sin(angle),0, cos(angle), 0,
        0,          0,          0, 1
    );
    */
}

//-------------------------------------------------------------------------------------------------------------
//Light GameObject  ------------------------------------------------------------------------------------------
Light::Light(int x, int y, int z, Color c, create::Engine* eng) {
    engine = eng;
    //TODO: need to have a check for the existence of a light object and do some default values if it doesnt exist

    //later on the size of the light cube will probably be ~0 (just a point in the scene)
    float cubeSize = (float) 0.2;

    float vulkanX = engine->screenToVulkan(x, engine->screenWidth, -1);
    float vulkanY = engine->screenToVulkan(y, engine->screenWidth, -1);

    std::vector<data::Vertex2D> vertices = {
        {{vulkanX - cubeSize, vulkanY - cubeSize, z, 0.0}, {c.red, c.green, c.blue}},
        {{vulkanX + cubeSize, vulkanY - cubeSize, z, 0.0}, {c.red, c.green, c.blue}},
        {{vulkanX + cubeSize, vulkanY + cubeSize, z, 0.0}, {c.red, c.green, c.blue}},
        {{vulkanX - cubeSize, vulkanY + cubeSize, z, 0.0}, {c.red, c.green, c.blue}},
        {{vulkanX - cubeSize, vulkanY - cubeSize, cubeSize * 2 + z, 0.0}, {c.red, c.green, c.blue}},
        {{vulkanX + cubeSize, vulkanY - cubeSize, cubeSize * 2 + z, 0.0}, {c.red, c.green, c.blue}},
        {{vulkanX + cubeSize, vulkanY + cubeSize, cubeSize * 2 + z, 0.0}, {c.red, c.green, c.blue}},
        {{vulkanX - cubeSize, vulkanY + cubeSize, cubeSize * 2 + z, 0.0}, {c.red, c.green, c.blue}}
    };

    //the indices will get add on to the end to m
    std::vector<uint16_t> indices =  {
        2, 3, 0, 0, 1, 2,
        6, 5, 4, 4, 7, 6,
        3, 7, 4, 4, 0, 3,
        7, 3, 2, 2, 6, 7,
        5, 6, 2, 2, 1, 5,
        4, 5, 1, 1, 0, 4
    };

    //std::cout  << "vertex count : " << vertexCount << std::endl;

    for (size_t i = 0; i < indices.size(); i++) {
        indices[i] += vertexCount;
    }

    //this code should be further abstracted, appears in every gameobject call
    indexCount += (uint16_t) indices.size();
    objectCount++;

    engine->writeToVertexBuffer(sizeof(vertices[0]) * vertices.size(), vertices.data());
    engine->writeToIndexBuffer(sizeof(indices[0]) * indices.size(), indices.data());

    //move to draw call probably
    //std::cout << indexCount << std::endl;
    engine->addToRenderQueue(indexCount);
}

Light::~Light() {}
