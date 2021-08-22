#include "model.hpp"

using namespace model;

Model::Model() {}
Model::~Model() {}

void Model::createModel(const std::string& fileName) {
	//read the object data with assimp
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(fileName, 
							aiProcess_CalcTangentSpace |
							aiProcess_JoinIdenticalVertices |
							aiProcess_Triangulate |
							aiProcess_SortByPType);

	aiNode* rootNode = scene->mRootNode;
	aiMesh** const sceneMeshes = scene->mMeshes;

	
	processScene(rootNode, sceneMeshes);
}

void Model::processScene(aiNode* node, aiMesh** const meshes) {
	printf("made it here \n");
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		mesh::Mesh newMesh = processMesh(meshes[node->mMeshes[i]]);

		printf("the size of this mesh now is: %u \n", newMesh.getIndexData());

		modelMeshes.push_back(newMesh);
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		processScene(node->mChildren[i], meshes);
	}
}

mesh::Mesh Model::processMesh(aiMesh* currentMesh) {
	//grab vertex data
	std::vector<data::Vertex> newVertices;
	for (unsigned int i = 0; i < currentMesh->mNumVertices; i++) {
		aiVector3D position = currentMesh->mVertices[i];
		aiVector3D normal = currentMesh->mNormals[i];

		data::Vertex vertex = {
			glm::vec4(position.x, position.y, position.z, 0.0),
			glm::vec4(normal.x, normal.y, normal.z, 0.0),
		};

		newVertices.push_back(vertex);
	}

	//grab index data
	std::vector<uint32_t> newIndices;
	for (unsigned int i = 0; i < currentMesh->mNumFaces; i++) {
		aiFace face = currentMesh->mFaces[i];
		if (face.mNumIndices == 3) {
			newIndices.push_back(face.mIndices[0]);
			newIndices.push_back(face.mIndices[1]);
			newIndices.push_back(face.mIndices[2]);
		}
	}

	//construct mesh
	mesh::Mesh newMesh(newVertices, newIndices);

	return newMesh;
}