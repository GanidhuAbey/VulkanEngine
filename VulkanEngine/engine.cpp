#include "engine.hpp"
#include "camera.hpp"
#include "data_formats.hpp"

#include <string>

using namespace create;

/*---------------- Engine -----------------*/
Engine::Engine(uint32_t width, uint32_t height, const char* title) {
	screenWidth = width;
	screenHeight = height;
	//inorder to initialize the engine we have to run the core function
    mainCamera = new UserCamera;

	engineCore.init(screenWidth, screenHeight, title);
    light.init(glm::vec3(0.0, 0.0, 0.0), Color(0.0, 0.0, 0.0));
}

void Engine::render() {
	//iterate over all the available objects
	for (size_t i = 0; i < objectData.size(); i++) {
		//we need to create a new buffer for this object
		UniformBufferObject ubo;
		ubo.modelToWorld = objectData[i]->transform;
		ubo.worldToCamera = mainCamera->worldToCamera;
		ubo.projection = mainCamera->perspective;
		
		//each object has its vertex, index, and uniform data but its not know whether this data is already attached or not
		if (!engineCore.hasUniformBuffer(i)) {
			//create vertex and index data
			std::vector<data::Vertex> vertices = objectData[i]->vertices;
			std::vector<uint16_t> indices = objectData[i]->indices;
            PushFragConstant pfc = objectData[i]->pfc;

			allObjectVertices.push_back(vertices);
			allObjectIndices.push_back(indices);
            allFragConstants.push_back(pfc);

			engineCore.writeToVertexBuffer(sizeof(vertices[0]) * vertices.size(), vertices.data());
			engineCore.writeToIndexBuffer(sizeof(indices[0]) * indices.size(), indices.data());

			//create uniform buffers to attach data to
			engineCore.attachData(ubo);

            engineCore.createCommands(allObjectIndices, allObjectVertices, light.light, allFragConstants);

            
		}
		engineCore.updateData(ubo, i);
	}
    engineCore.draw();
}

Engine::~Engine() {}

/*---------------- Window Functionality -----------------*/
void Engine::captureCursor() {
	engineCore.userWindow.captureCursor();
}

bool Engine::checkCloseRequest() {
	return engineCore.userWindow.closeRequest();
}

void Engine::getCursorPosition(double* xPos, double* yPos) {
	engineCore.userWindow.getCursorPosition(xPos, yPos);
}

bool Engine::isKeyPressed(WindowInput input) {
	return engineCore.userWindow.isKeyPressed(input);
}

/*---------------- Camera Creation -----------------*/
UserCamera* Engine::createCamera(glm::vec3 cameraPosition, glm::vec3 cameraDirection, glm::vec3 cameraOrientation, float vertical_fov) {
	mainCamera->init(cameraPosition, cameraDirection, cameraOrientation, vertical_fov, screenWidth/(float)screenHeight);

	return mainCamera;
}

void UserCamera::init(glm::vec3 cameraPosition, glm::vec3 cameraDirection, glm::vec3 cameraOrientation, float vertical_fov, float aspect_ratio) {
	//create camera related matrices
	worldToCamera = camera::createCameraMatrix(cameraPosition, cameraPosition + cameraDirection, cameraOrientation);
	perspective = camera::createPerspectiveMatrix(vertical_fov, aspect_ratio, 0.1f, 150.0f);

	//update current camera values
	eye = cameraPosition;
	target = cameraPosition + cameraDirection;
	up = cameraOrientation;

}
void UserCamera::update(std::optional<glm::vec3> position, std::optional<glm::vec3> direction, std::optional<glm::vec3> camera_up) {
	eye = (position.has_value()) ? position.value() : eye;
	target = (direction.has_value()) ? (eye + direction.value()) : target;
	up = (camera_up.has_value()) ? camera_up.value() : up;

	worldToCamera = camera::createCameraMatrix(eye, target, up);
}

UserCamera::~UserCamera() {}


/*---------------- Object Creation -----------------*/
UserObject* Engine::createObject() {
	UserObject* obj = new UserObject;
	obj->init(objectData.size());

	objectData.push_back(obj);

	return objectData[objectData.size() - 1];
}

void UserObject::init(size_t objectIndex) {
	//initially objects will be defined with not transformation so their transform matrix would be just an identity matrix
	transform = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1,
	};

	index = objectIndex;
}

void UserObject::translate(glm::vec3 translate_vector) {
    glm::mat4 translate = {
        1, 0, 0, translate_vector.x,
        0, 1, 0, translate_vector.y,
        0, 0, 1, translate_vector.z,
        0, 0, 0, 1,
    };

    transform = glm::transpose(translate) * transform;
}

void UserObject::scale(float x, float y, float z) {
    glm::mat4 scale = {
        x, 0, 0, 0,
        0, y, 0, 0,
        0, 0, z, 0,
        0, 0, 0, 1,
    };

    transform =  transform * glm::transpose(scale);
}

void UserObject::addMesh(const std::string& fileName, Color c) {
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

    std::vector<glm::vec4> vertice_data;
    std::vector<glm::vec4> normal_data;

    //when vt and vn are needed we can just add more variables here
    int vCount = 0;
    int vLineCount = 0;

    int vnCount = 0;
    int vnLineCount = 0;

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
            size_t v_size = v.size();
            if (v_size % 3 == 0) {
                //a vec3 can be formed now
                glm::vec4 vertice = glm::vec4(v[v_size - 3], v[v_size - 2], v[v_size - 1], 0.0);
                vertice_data.push_back(vertice);
            }
            value = "";
        }

        if (name == "vn" && number && found) {
            vnCount++;
            vn.push_back(std::stof(value));
            size_t vn_size = vn.size();
            if (vn_size % 3 == 0) {
                glm::vec4 normal = glm::vec4(vn[vn_size - 3], vn[vn_size - 2], vn[vn_size - 1], 0.0);
                normal_data.push_back(normal);
            }
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
            delimiter = "\n";
            pos = value.find(delimiter);
            std::string token = value.substr(0, pos);
            index_types[count] = static_cast<uint16_t>(std::stoul(token));
            value.erase(0, pos + delimiter.length());

            uint16_t normal_index = index_types[2] - 1;
            uint16_t vertex_index = index_types[0] - 1;
            indices.push_back(vertex_index);
            normalIndices.push_back(normal_index);

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
            if (name == "v") vLineCount++;
            if (name == "vn") vnLineCount++;

            name = "";
            value = "";
            number = false;
            found = false;
        }

    }

    std::vector<uint16_t> newIndices;
    for (size_t i = 0; i < indices.size(); i++) {
        glm::vec4 vertice = vertice_data[indices[i]];
        glm::vec4 normal = normal_data[normalIndices[i]];

        data::Vertex vertex{};
        vertex.position = vertice;
        vertex.normal = normal;

        vertices.push_back(vertex);
        newIndices.push_back(static_cast<uint16_t>(i));
        //printf("%u \n", i);
    }

    indices = newIndices;

    pfc.color = glm::vec4(c.red, c.green, c.blue, 0.0);
}

UserObject::~UserObject() {}


/*------------------ Light Creation --------------------*/
LightSource* Engine::createLight(glm::vec3 pos, Color lightColor) {
    UserObject* lightObject = createObject();
    lightObject->addMesh("objects/test_object/test.obj", lightColor);
    lightObject->translate(pos);
    lightObject->scale(0.1, 0.1, 0.1);

    light.update(pos, lightColor);

    return &light;
}

void LightSource::init(glm::vec3 pos, Color lightColor) {
    //lets quickly add some small mesh data to this object just for fun.
    light.color = glm::vec4(lightColor.red, lightColor.green, lightColor.blue, 0.0);
    light.position = glm::vec4(pos, 0.0);
}

void LightSource::update(std::optional<glm::vec3> pos, std::optional<Color> lightColor) {
    light.color = (lightColor.has_value()) ? glm::vec4(lightColor.value().red, lightColor.value().green, lightColor.value().blue, 0.0) : light.color;
    light.position = (pos.has_value()) ? glm::vec4(pos.value(), 0.0) : light.position;
}

LightSource::~LightSource() {}