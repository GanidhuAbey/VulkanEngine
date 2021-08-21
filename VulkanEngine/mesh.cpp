#include "mesh.hpp"

#include <stdexcept>

using namespace mesh;


Mesh::Mesh() {
	//create an empty mesh
	data::Vertex vertex = {
		glm::vec4(0.0, 0.0, 0.0, 0.0),
		glm::vec4(0.0, 0.0, 0.0, 0.0),
	};
	vertices.push_back(vertex);

	indices.push_back(0);
	indices.push_back(0);
	indices.push_back(0);
}

Mesh::~Mesh() {}

void Mesh::addMesh(const std::string& fileName) {
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(fileName,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	if (!scene) throw std::runtime_error("could not import scene");

	aiNode* rootNode = scene->mRootNode;
	aiMesh** const sceneMeshes = scene->mMeshes;

	std::vector<data::Vertex> newVertices = accessDataVert(rootNode, sceneMeshes, newVertices);
	uint32_t vertexOffset = 0;
	std::vector<uint32_t> newIndices = accessDataIndex(rootNode, sceneMeshes, newIndices, &vertexOffset);

	vertices.resize(newVertices.size());
	vertices = newVertices;
	
	indices.resize(newIndices.size());
	indices = newIndices;

	printf("----------------------------------------- \n");
	for (size_t i = 0; i < indices.size(); i+=3) {
		printf("< %u | %u | %u > \n", indices[i], indices[i + 1], indices[i + 2]);
	}

}

data::Vertex Mesh::createVertexFromAssimp(aiMesh* mesh, unsigned int index) {
	aiVector3D position = mesh->mVertices[index];
	aiVector3D normal = mesh->mNormals[index];

	data::Vertex vertex = {
		glm::vec4(position.x, position.y, position.z, 0.0),
		glm::vec4(normal.x, normal.y, normal.z, 0.0)
	};

	return vertex;
}

std::vector<data::Vertex> Mesh::accessDataVert(aiNode* node, aiMesh** const meshes, std::vector<data::Vertex> vertices) {
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

std::vector<uint32_t> Mesh::accessDataIndex(aiNode* node, aiMesh** const meshes, std::vector<uint32_t> indices, uint32_t* meshOffset) {
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

std::vector<data::Vertex> Mesh::getVertexData() {
	return vertices;
}

std::vector<uint32_t> Mesh::getIndexData() {
	return indices;
}