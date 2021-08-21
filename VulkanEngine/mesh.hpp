#pragma once

#include "data_formats.hpp"

#include <vector>
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags


namespace mesh {
class Mesh {
private:
	std::vector<data::Vertex> vertices;
	std::vector<uint32_t> indices;

public:
	//take the vertex data and load it into a buffer
	Mesh();
	~Mesh();
	void addMesh(const std::string& fileName);

private:
	std::vector<data::Vertex> accessDataVert(aiNode* node, aiMesh** const meshes, std::vector<data::Vertex> vertices);
	std::vector<uint32_t> accessDataIndex(aiNode* node, aiMesh** const meshes, std::vector<uint32_t> indices, uint32_t* meshOffset);

	data::Vertex createVertexFromAssimp(aiMesh* mesh, unsigned int index);

public:
	std::vector<data::Vertex> getVertexData();
	std::vector<uint32_t> getIndexData();
};
}