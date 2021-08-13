#include "game_objects.hpp"
#include "common.hpp"
#include "math.h"
#include <string>

using namespace gameObject;


const int DEPTH_CONSTANT = 100;

std::tuple<std::vector<data::Vertex>, std::vector<uint16_t>> gameObject::readObjData(const std::string& fileName) {
    //convert mesh data into vertices and index data.
    std::vector<char> meshData = readFile(fileName);

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

    std::vector<uint16_t> indices;
    std::vector<data::Vertex> vertices;

    //when vt and vn are needed we can just add more variables here
    int vCount = 0;
    int vLineCount = 0;

    for (size_t i = 0; i < meshData.size(); i++) {
        char data = meshData[i];
        if (found && data != ' ' && data != '\n') {
            value += data;
        }
        else {
            number = true;
        }

        //generated a value now we determine where to add it
        if (name == "v" && number && found) {
            vCount++;
            v.push_back(std::stof(value));
            value = "";
        }

        if (name == "f" && number && found) {
            std::string delimiter = "/";
            size_t count = 0;
            size_t pos = 0;
            std::vector<uint16_t> index_types(3); // im gonna regret putting a constant here ...
            while ((pos = value.find(delimiter)) != std::string::npos) {
                std::string token = value.substr(0, pos);
                index_types[count] = static_cast<uint16_t>(std::stoul(token));
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
            //printf("new line statement \n");
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

    //last step is to convert them to our format
    for (size_t i = 0; i < v.size() / vDataCount; i++) {
        //for now i only accept sizes of 3
        if (vDataCount != 3) {
            printf("WARNING: obj file containts vector-%zu but expected vector-3", vDataCount);
        }

        //now we use this to create the vec4 we need
        size_t index = 3 * i;
        glm::vec4 vertice = glm::vec4(v[index], v[index + 1], v[index + 2], 0.0);

        //materials haven't been implemented so we'll just do a default value here
        glm::vec3 color = glm::vec3(0.1, 0.2, 0.6);

        data::Vertex vertex = {
            vertice,
            color
        };

        vertices.push_back(vertex);
    }
    return std::make_tuple(vertices, indices);
}