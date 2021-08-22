#pragma once

#include "mesh.hpp"

namespace model {

class Model {
public:
	std::vector<mesh::Mesh> modelMeshes;
public:
	Model();
	~Model();
	void createModel(const std::string& fileName);

private:
	std::vector<data::Vertex> accessDataVert(aiNode* node, aiMesh** const meshes, std::vector<data::Vertex> vertices);
	std::vector<uint32_t> accessDataIndex(aiNode* node, aiMesh** const meshes, std::vector<uint32_t> indices, uint32_t* meshOffset);
	std::vector<std::vector<glm::vec4>> accessDataTextures(aiNode* node, aiMesh** const meshes, std::vector<std::vector<glm::vec4>> textures);
	data::Vertex createVertexFromAssimp(aiMesh* mesh, unsigned int index);

	void processScene(aiNode* node, aiMesh** const meshes);
	mesh::Mesh processMesh(aiMesh* currentMesh);
};

}