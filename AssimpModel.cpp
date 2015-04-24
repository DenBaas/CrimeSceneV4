#include "AssimpModel.h"

#include <CaveLib\Shader.h>
#include <CaveLib\VBO.h>
#include <CaveLib\Vertex.h>
#include <Assimp\Importer.hpp>
#include <Assimp\postprocess.h>
#include <Assimp\scene.h>

#include <exception>
#include <iostream>
#include <fstream>

#include "CrimeScene.h"

/*
Constructor
Imports a model via Assimp, and then loads it
folderName = location of the folder to load the model from
fileName = name of the model, with its extension
loadedTextures = pointer to a map of strings and cTextures that contains the already loaded textures
Author: Bas Rops - 08-05-2014
Last edit: Bas Rops - 27-05-2014
*/
AssimpModel::AssimpModel(std::string folderName, std::string fileName, float scale, std::map<std::string, cTexture*>* loadedTextures)
{
	try
	{
		model = importModel(folderName + fileName);

		aiMatrix4x4 parentMatrix = aiMatrix4x4();
		loadModel(folderName, model, model->mRootNode, scale, parentMatrix, loadedTextures);
		//Calculate the boundingbox of the full model
		for (unsigned int i = 0; i < meshes.size(); i++)
		{
			glm::vec3 meshBoundingBoxMin = meshes[i]->getBoundingBox().mMin;
			glm::vec3 meshBoundingBoxMax = meshes[i]->getBoundingBox().mMax;

			if (i > 0)
			{
				this->boundingBox.mMin.x = glm::min(this->boundingBox.mMin.x, meshBoundingBoxMin.x);
				this->boundingBox.mMin.y = glm::min(this->boundingBox.mMin.y, meshBoundingBoxMin.y);
				this->boundingBox.mMin.z = glm::min(this->boundingBox.mMin.z, meshBoundingBoxMin.z);

				this->boundingBox.mMax.x = glm::max(this->boundingBox.mMax.x, meshBoundingBoxMax.x);
				this->boundingBox.mMax.y = glm::max(this->boundingBox.mMax.y, meshBoundingBoxMax.y);
				this->boundingBox.mMax.z = glm::max(this->boundingBox.mMax.z, meshBoundingBoxMax.z);
			}
			else
			{
				this->boundingBox.mMin = meshBoundingBoxMin;
				this->boundingBox.mMax = meshBoundingBoxMax;
			}					
		}

		//Only show the amount of non-triangles of the model in debug mode
#ifdef _DEBUG
		int nonTriangles = 0;
		for each (AssimpMesh* mesh in meshes)
		{
			nonTriangles += mesh->getNonTriangles();
		}
		std::cout << "Amount of non-triangles in model " << fileName << ": " << nonTriangles << std::endl;
#endif
	}
	catch (std::exception& ex)
	{
		std::cout << "Error while importing model " << fileName << ". Exception: " << ex.what() << std::endl;
	}
}

/*
Destructor
Author: Bas Rops - 08-05-2014
Last edit: Bas Rops 13-06-2014
*/
AssimpModel::~AssimpModel()
{
	meshes.clear();
}

/*
Import a model using Assimp
fileName = location of the model to import
Author: Bas Rops - 08-05-2014
Last edit: Bas Rops - 09-05-2014
*/
const aiScene* AssimpModel::importModel(std::string fileName)
{
	//Check if file exists
	std::ifstream fin(fileName.c_str());
	if (!fin.fail())
		fin.close();
	else
	{
		throw(std::exception(("Could not open file " + fileName).c_str()));
		return NULL;
	}

	const aiScene* tmpModel = importer.ReadFile(fileName, aiProcessPreset_TargetRealtime_Quality);

	//If the import failed, report it
	if (!tmpModel)
	{
		throw(std::exception(importer.GetErrorString()));
		return NULL;
	}

	// We're done. Everything will be cleaned up by the importer destructor
	return tmpModel;
}

/*
Load the meshes of the imported model
folderName = location of the folder to load the model from
model = pointer to the model that is being loaded
node = pointer to the node that is being loaded
loadedTextures = pointer to a map of strings and cTextures that contains the already loaded textures
Author: Bas Rops - 08-05-2014
Last edit: Bas Rops - 27-05-2014
*/
void AssimpModel::loadModel(std::string folderName, const aiScene* model, const aiNode* node, float scale, aiMatrix4x4 parentMatrix, std::map<std::string, cTexture*>* loadedTextures)
{
	//Multiply the parentMatrix with the node matrix, so that a "tree" of matrices is created
	aiMatrix4x4 meshMatrix = parentMatrix * node->mTransformation;

	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		const struct aiMesh* mesh = this->model->mMeshes[node->mMeshes[i]];

		meshes.push_back(new AssimpMesh(folderName, model, mesh, meshMatrix, scale, loadedTextures));
	}
		
	//Load all children
	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		loadModel(folderName, model, node->mChildren[i], scale, meshMatrix, loadedTextures);
	}
}

/*
Retrieve the boundingbox of this model
Author: Bas Rops - 20-05-2014
Last edit: <name> - dd-mm-yyyy
*/
Bbox AssimpModel::getBoundingBox()
{
	return this->boundingBox;
}

/*
Draw the model by drawing every mesh
Author: Bas Rops - 08-05-2014
Last edit: Bas Rops - 09-05-2014
*/
void AssimpModel::draw(Shader<CrimeScene::Uniforms>* shader, glm::mat4* modelMatrix)
{
	for each (AssimpMesh* mesh in meshes)
	{
		mesh->draw(shader, modelMatrix);
	}
}
