#include "model.hpp"

#include <stdexcept>

using namespace model;

Model::Model() {}
Model::~Model() {}

void Model::createModel(const std::string& fileName) {
	//read the object data with assimp
	
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(fileName,
							aiProcess_GenNormals |
							aiProcess_CalcTangentSpace |
							aiProcess_JoinIdenticalVertices |
							aiProcess_Triangulate |
							aiProcess_SortByPType);

	aiNode* rootNode = scene->mRootNode;
	aiMesh** const sceneMeshes = scene->mMeshes;
	aiMaterial** sceneMaterials = scene->mMaterials;

	//for now we will just throw an error when texture data isn't present
	//TODO: change this later
	if (scene->mNumMaterials == 0) {
		throw std::runtime_error("unfortunately this object does not contain any texture data");
	}
	
	processScene(rootNode, sceneMeshes, sceneMaterials);

	//printf("this size of model meshes is: %zu \n", modelMeshes.size());
}

void Model::processScene(aiNode* node, aiMesh** const meshes, aiMaterial** materials) {
	printf("made it here \n");
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		mesh::Mesh newMesh = processMesh(meshes[node->mMeshes[i]], materials);
		newMesh.getTexturePaths();
		modelMeshes.push_back(newMesh);
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		processScene(node->mChildren[i], meshes, materials);
	}
}

mesh::Mesh Model::processMesh(aiMesh* currentMesh, aiMaterial** materials) {
	//grab vertex data
	std::vector<data::Vertex> newVertices;
	std::vector<std::vector<glm::vec3>> textureData;
	for (unsigned int i = 0; i < currentMesh->mNumVertices; i++) {
		aiVector3D position = currentMesh->mVertices[i];
		aiVector3D normal = currentMesh->mNormals[i];
		
		//grab texture data?
		glm::vec2 texCoord;
		if (currentMesh->mTextureCoords[0]) {
			texCoord.x = currentMesh->mTextureCoords[0][i].x;
			texCoord.y = currentMesh->mTextureCoords[0][i].y;
		} 
		else texCoord = glm::vec2(0.0, 0.0);

		data::Vertex vertex = {
			glm::vec4(position.x, position.y, position.z, 0.0),
			glm::vec4(normal.x, normal.y, normal.z, 0.0),
			texCoord,
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

	//grab texture data?
	aiMaterial* mat = materials[currentMesh->mMaterialIndex];
	unsigned int texCount = mat->GetTextureCount(aiTextureType_DIFFUSE);
	aiString texturePath;
	char imagePath;
	std::vector<aiString> texturePaths;
	for (unsigned int i = 0; i < texCount; i++) {
		mat->GetTexture(aiTextureType_DIFFUSE, i, &texturePath);
		texturePaths.push_back(texturePath);
		//printf("the image path: %s \n", &imagePath);
		
		printf("texture size over here: %u \n", texturePaths.size());
	}

	//construct mesh
	printf("texture size ssshere: %s \n", texturePaths[0].data);
	mesh::Mesh newMesh(newVertices, newIndices, texturePaths);

	return newMesh;
}

void Model::generateTextures(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t queueFamilyIndexCount, uint32_t* pQueueFamilyIndices) {
}