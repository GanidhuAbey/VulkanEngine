#pragma once

#include "core.hpp"

#include <vector>
#include <glm/glm.hpp>

//This file will act as the part of the library the user will interact with

namespace create {
class Color {
public:
	float red;
	float green;
	float blue;

public:
	Color(float r, float g, float b) : red{ r }, green{ g }, blue{ b } {};
};


class LightSource {
public:
	LightObject light;
public:
	void init(glm::vec3 pos, Color lightColor);
	void update(std::optional<glm::vec3> pos = std::nullopt, std::optional<Color> lightColor = std::nullopt);
	~LightSource();
};

class UserObject {
private:
	size_t index;
	std::vector<uint16_t> normalIndices;
public:
	glm::mat4 transform;
	std::vector<data::Vertex> vertices;
	std::vector<uint16_t> indices;
	
	PushFragConstant pfc;

public:
	void init(size_t objectIndex);
	~UserObject();
public:
	void addMesh(const std::string& fileName, Color c);
	void translate(glm::vec3 translate_vector);
	void scale(float x, float y, float z);
};

class UserCamera {
private:
	glm::vec3 eye;
	glm::vec3 target;
	glm::vec3 up;
public:
	glm::mat4 worldToCamera;
	glm::mat4 perspective;

public:
	void init(glm::vec3 cameraPosition, glm::vec3 cameraDirection, glm::vec3 cameraOrientation, float vertical_fov, float aspect_ratio);
	~UserCamera();
public:
	void update(std::optional<glm::vec3> position = std::nullopt, std::optional<glm::vec3> direction = std::nullopt, std::optional<glm::vec3> camera_up = std::nullopt);
};

class Engine {
private:
	std::vector<UserObject*> objectData;
	std::vector<std::vector<data::Vertex>> allObjectVertices;
	std::vector<std::vector<uint16_t>> allObjectIndices;
	std::vector<PushFragConstant> allFragConstants;

	UserCamera* mainCamera;
	core::Core engineCore;

	uint32_t screenWidth;
	uint32_t screenHeight;

	LightSource light;

public:
	Engine(uint32_t width, uint32_t height, const char* title);
	~Engine();

public:
	UserObject* createObject();
	UserCamera* createCamera(glm::vec3 cameraPosition, glm::vec3 cameraDirection, glm::vec3 cameraOrientation, float vertical_fov);
	LightSource* createLight(glm::vec3 pos, Color lightColor);

	void captureCursor();
	bool checkCloseRequest();
	void getCursorPosition(double* xPos, double* yPos);
	bool isKeyPressed(WindowInput input);

	void render();
};
}