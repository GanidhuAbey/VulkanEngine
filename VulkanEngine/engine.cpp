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
			mesh::Mesh currentMesh = objectData[i]->objectMesh;
            PushFragConstant pfc = objectData[i]->pfc;

            allFragConstants.push_back(pfc);

			allMeshData.push_back(currentMesh);

			std::vector<data::Vertex> verts = currentMesh.getVertexData();
			std::vector<uint32_t> indexes = currentMesh.getIndexData();

			engineCore.writeToVertexBuffer(verts);
			engineCore.writeToIndexBuffer(indexes);

			//create uniform buffers to attach data to
			engineCore.attachData(ubo);

            engineCore.createCommands(allMeshData, light.light, allFragConstants);

            
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
    //just going to use the assimp library for this
	objectMesh.addMesh(fileName);	
	
	//now we have both vertices and indices
	pfc.color = glm::vec4(c.red, c.green, c.blue, 0.0);

}

data::Vertex create::createVertexFromAssimp(aiMesh* mesh, unsigned int index) {
	aiVector3D position = mesh->mVertices[index];
	aiVector3D normal = mesh->mNormals[index];

	data::Vertex vertex = {
		glm::vec4(position.x, position.y, position.z, 0.0),
		glm::vec4(normal.x, normal.y, normal.z, 0.0)
	};

	return vertex;
}

std::vector<data::Vertex> create::accessDataVert(aiNode* node, aiMesh** const meshes, std::vector<data::Vertex> vertices) {
	unsigned int meshCount = node->mNumMeshes;

	for (unsigned int i = 0; i < meshCount; i++) {
		aiMesh* currentMesh = meshes[node->mMeshes[i]];
		unsigned int vertexCount = currentMesh->mNumVertices;
		for (unsigned int j = 0; j < vertexCount; j++) {
			vertices.push_back(createVertexFromAssimp(currentMesh, j));
		}
		//*meshOffset = *meshOffset + currentMesh->mNumVertices;
	}

	for (unsigned int k = 0; k < node->mNumChildren; k++) {
		auto it = vertices.end();
		std::vector<data::Vertex> vertices_branch = accessDataVert(node->mChildren[k], meshes, vertices_branch);
		vertices.insert(it, vertices_branch.begin(), vertices_branch.end());
	}

	return vertices;
}

std::vector<uint32_t> create::accessDataIndex(aiNode* node, aiMesh** const meshes, std::vector<uint32_t> indices, uint32_t* meshOffset) {
	unsigned int meshCount = node->mNumMeshes;
	
	for (unsigned int i = 0; i < meshCount; i++) {
		aiMesh* currentMesh = meshes[node->mMeshes[i]];
		unsigned int indexCount = currentMesh->mNumFaces;
		for (unsigned int j = 0; j < indexCount; j++) {
			aiFace face = currentMesh->mFaces[j];
			if (face.mNumIndices == 3) {
				indices.push_back(face.mIndices[0] + *meshOffset);
				indices.push_back(face.mIndices[1] + *meshOffset);
				indices.push_back(face.mIndices[2] + *meshOffset);
			}
		}
		*meshOffset = *meshOffset + currentMesh->mNumVertices;
	}

	for (unsigned int k = 0; k < node->mNumChildren; k++) {
		auto it = indices.end();
		std::vector<uint32_t> index_branch = accessDataIndex(node->mChildren[k], meshes, index_branch, meshOffset);
		indices.insert(it, index_branch.begin(), index_branch.end());
	}

	return indices;
}

void create::accessIndices(aiNode* node, aiMesh** const meshes, uint32_t* meshIndices, uint32_t* p, uint32_t* offset, uint32_t* meshOffset, bool writeTo) {
	unsigned int meshCount = node->mNumMeshes;

	unsigned int offsetBy = 0;
	for (unsigned int i = 0; i < meshCount; i++) {
		aiMesh* currentMesh = meshes[node->mMeshes[i]];
		unsigned int indicesCount = currentMesh->mNumFaces;
		unsigned int indexOffset = 0;
		unsigned int currentDex = 0;
		for (unsigned int j = 0; j < indicesCount; j++) {
			aiFace face = currentMesh->mFaces[j];
			if (writeTo && face.mNumIndices == 3) {
				//printf("the index being accessed is : %u \n", *offset + meshOffset + j);
				//currentDex = *offset + meshOffset + indexOffset + j;
				//printf("!| %u | \n", meshOffset);
				meshIndices[*offset] = face.mIndices[0] + *meshOffset;
				meshIndices[*offset + 1] = face.mIndices[1] + *meshOffset;
				meshIndices[*offset + 2] = face.mIndices[2] + *meshOffset;
				indexOffset += 2;
			}
			else if (face.mNumIndices == 3) {
				//printf("~ | %u | \n", *p);
				*p = *p + 3;
			}

			if (face.mNumIndices == 3) {
				*offset = *offset + 3;
			}
		}
		//meshOffset = currentDex + 3;
		*meshOffset = *meshOffset + currentMesh->mNumVertices;
	}
	
	//*offset = meshOffset;
	//printf("[==================================================] \n");
	for (size_t k = 0; k < node->mNumChildren; k++) {
		//printf("hello was up: %u \n", meshes[node->mChildren[k]->mMeshes[0]]->mFaces[0].mIndices[1]);
		accessIndices(node->mChildren[k], meshes, meshIndices, p, offset, meshOffset, writeTo);
	}
}

void create::accessVertices(aiNode* node, aiMesh** const meshes, data::Vertex* positions, uint32_t* p, uint32_t* offset, bool writeTo) {
	unsigned int meshCount = node->mNumMeshes;
	unsigned int meshOffset = 0;

	for (unsigned int i = 0; i < meshCount; i++) {
		aiMesh* currentMesh = meshes[node->mMeshes[i]];
		unsigned int verticesCount = currentMesh->mNumVertices;
		for (unsigned int j = 0; j < verticesCount; j++) {
			if (writeTo) {
				//record vertices from mesh
				aiVector3D normal = currentMesh->mNormals[j];
				aiVector3D position = currentMesh->mVertices[j];
				
				//printf("<%f %f %f> \n", position.x, position.y, position.z);
				//printf("indexes: %u \n", *offset);
				glm::vec4 convertedPosition = glm::vec4(position.x, position.y, position.z, 0.0);
				positions[*offset].position = convertedPosition;
				positions[*offset].normal = glm::vec4(normal.x, normal.y, normal.z, 0.0);
			}
			else {
				//printf("indice: %u \n", *p);
				*p = *p + 1;
			}
			*offset = *offset + 1;
		}
		meshOffset += verticesCount;
	}	

	//*offset = *offset + meshOffset;
	for (size_t j = 0; j < node->mNumChildren; j++) {
		accessVertices(node->mChildren[j], meshes, positions, p, offset, writeTo);
	}
}



//A
//C
//B
//A
//C
//B


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