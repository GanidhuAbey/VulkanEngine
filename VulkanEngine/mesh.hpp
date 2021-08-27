#pragma once

#include "data_formats.hpp"
#include "memory_allocator.hpp"

#include <vector>
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags


namespace mesh {
class Mesh {
private:
	std::vector<data::Vertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<aiString> texturePaths;

	//maybe organize this into a structure...
	mem::MaMemory texture;
	uint32_t textureWidth;
	uint32_t textureHeight;

public:
	//take the vertex data and load it into a buffer
	Mesh(std::vector<data::Vertex> newVertices, std::vector<uint32_t> newIndices, std::vector<aiString> textureData);
	~Mesh();
	void freeMesh(VkDevice device);
	void addMesh(const std::string& fileName);
	void addTexture(mem::MaMemory textureData);

private:
	std::vector<data::Vertex> accessDataVert(aiNode* node, aiMesh** const meshes, std::vector<data::Vertex> vertices);
	std::vector<uint32_t> accessDataIndex(aiNode* node, aiMesh** const meshes, std::vector<uint32_t> indices, uint32_t* meshOffset);
	std::vector<std::vector<glm::vec4>> accessDataTextures(aiNode* node, aiMesh** const meshes, std::vector<std::vector<glm::vec4>> textures);
	data::Vertex createVertexFromAssimp(aiMesh* mesh, unsigned int index);
	std::vector<std::vector<glm::vec3>> loadTextureFromPath(char* texturePath);

public:
	std::vector<data::Vertex> getVertexData();
	std::vector<uint32_t> getIndexData();
	std::vector<aiString> getTexturePaths();
	mem::MaMemory* getTextureData();
	void saveDimensions(uint32_t width, uint32_t height);
	uint32_t getTextureWidth();
	uint32_t getTextureHeight();
};
}