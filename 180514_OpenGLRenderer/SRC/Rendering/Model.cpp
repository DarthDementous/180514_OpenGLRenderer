#include "Model.h"
#include "Mesh.h"
#include "TextureWrapper.h"
#include "VertexFormat.h"
#include "Transform.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <glm/vec4.hpp>

Model::Model(const char * a_filePath)
{
	// Initialize variables
	m_modelTransform = new Transform();

	LoadModel(a_filePath);
}

Model::~Model()
{
	// Delete all loaded textures
	for (int i = 0; i < m_loadedTextures.size(); ++i) {
		delete m_loadedTextures[i];
	}

	// Delete all meshes
	for (int i = 0; i < m_meshes.size(); ++i) {
		delete m_meshes[i];
	}
}

void Model::Draw(RenderCamera * a_camera, std::vector<PhongLight*> a_lights, const glm::vec4 & a_globalAmbient, ShaderWrapper * a_ambientPass, ShaderWrapper * a_directionalPass, ShaderWrapper * a_pointPass, ShaderWrapper * a_spotPass)
{
	// Draw all meshes
	for (int i = 0; i < m_meshes.size(); ++i) {
		m_meshes[i]->Draw(a_camera, a_lights, a_globalAmbient, a_ambientPass, a_directionalPass, a_pointPass, a_spotPass);
	}
}

void Model::SetRotation(const glm::vec3 & a_rot)
{
	// Set rotation of model transform
	m_modelTransform->SetRotation(a_rot);

	// Set rotation of child mesh transforms
	for (int i = 0; i < m_meshes.size(); ++i) {
		m_meshes[i]->GetTransform()->SetRotation(a_rot);
	}
}

void Model::LoadModel(std::string a_filePath)
{
	Assimp::Importer modelImporter;

	// Load model 'scene' from file path (encompassing data for whole model including root node, meshes and materials)
	const aiScene* modelScene = modelImporter.ReadFile(a_filePath, aiProcess_Triangulate);// | aiProcess_FlipUVs);		// Model importer will ensure all faces are triangles and that uvs are flipped where necessary

	// Error handling
	try {
		if (!modelScene || modelScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !modelScene->mRootNode) {	// Failed to load model completely or no root node
			char errorMsg[256];
			sprintf_s(errorMsg, "ERROR::ASSIMP:: %s", modelImporter.GetErrorString());

			throw std::runtime_error(errorMsg);
		}
	}
	catch (std::exception const& e) { std::cout << "Exception: " << e.what() << std::endl; }

	// Determine model folder directory and store
	m_modelDirectory = a_filePath.substr(0, a_filePath.find_last_of('/'));		// Model directory = sub-string after last backslash e.g. "models/players/boy/boy.fbx" = "models/players/boy"
	
	// Recursively process all nodes in model scene by starting at root node
	RecurReadNode(modelScene->mRootNode, modelScene);
}

/**
*	@brief Recursively process all meshes inside the model starting from a given root node.
*	@param a_node is the current node being processed.
*	@param a_modelScene is the overall data of the model.
*	@return void.
*/
void Model::RecurReadNode(aiNode * a_node, const aiScene * a_modelScene)
{
	// NOTE: Assimp stores all the actual data in the model scene and then references to that data inside of the nodes for efficiency

	/// Process node's meshes
	for (unsigned int i = 0; i < a_node->mNumMeshes; ++i) {
		aiMesh* mesh = a_modelScene->mMeshes[a_node->mMeshes[i]];	// Get mesh corresponding to current mesh indice in node's meshes (horrible naming conventions but oh well)
		m_meshes.push_back(ReadMesh(mesh, a_modelScene));
	}

	/// Process node's children
	for (unsigned int i = 0; i < a_node->mNumChildren; ++i) {
		RecurReadNode(a_node->mChildren[i], a_modelScene);
	}
}

/**
*	@brief Read in vertex, indice and material data from mesh in order to create and return a mesh object.
*	@brief a_mesh is the mesh to read from.
*	@brief a_modelScene is the overall data holder for the model.
*	@return constructed Mesh object based on the read in data.
*/
Mesh* Model::ReadMesh(aiMesh * a_mesh, const aiScene * a_modelScene)
{
	/// Mesh paramaters to be read into
	std::vector<Vertex>				readVertices;
	std::vector<unsigned int>		readIndices;
	std::vector<TextureWrapper*>	readTextures;

	/// Read in vertices
#pragma region
	for (unsigned int i = 0; i < a_mesh->mNumVertices; ++i) {
		// Position
		glm::vec4 posData;
		posData.x = a_mesh->mVertices[i].x;
		posData.y = a_mesh->mVertices[i].y;
		posData.z = a_mesh->mVertices[i].z;
		posData.w = 1.f;

		// Texture coordinates
		glm::vec2 texCoordData = glm::vec2(0.f);
		if (a_mesh->mTextureCoords[0]) {		// Mesh does contain texture coordinates
			// NOTE: Assimp allows for 8 different sets of texture coordinates per vertex, but most models only have one set (index [0])
			texCoordData.x = a_mesh->mTextureCoords[0][i].x;
			texCoordData.y = a_mesh->mTextureCoords[0][i].y;

		}

		// Normals
		glm::vec4 normalData;
		normalData.x = a_mesh->mNormals[i].x;
		normalData.y = a_mesh->mNormals[i].y;
		normalData.z = a_mesh->mNormals[i].z;
		normalData.w = 0.f;

		// Construct vertex from read in data and add to vector
		readVertices.push_back(Vertex(posData, texCoordData, normalData));
	}

#pragma endregion

	/// Read in indices
	for (unsigned int i = 0; i < a_mesh->mNumFaces; ++i) {
		// NOTE: Indices are stored in faces
		aiFace face = a_mesh->mFaces[i];

		for (unsigned int j = 0; j < face.mNumIndices; ++j) {
			readIndices.push_back(face.mIndices[j]);
		}
	}

	/// Read in textures
	// Create struct to store material information
	Material materialInfo;

	if (a_mesh->mMaterialIndex >= 0) {	// Mesh contains a material (one material per mesh)
		aiMaterial* material = a_modelScene->mMaterials[a_mesh->mMaterialIndex];

		// Load diffuse maps and append them onto read textures
		std::vector<TextureWrapper*> diffuseMaps = ReadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		readTextures.insert(readTextures.end(), diffuseMaps.begin(), diffuseMaps.end());
		
		// Load specular maps and append them onto read textures
		std::vector<TextureWrapper*> specularMaps = ReadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		readTextures.insert(readTextures.end(), specularMaps.begin(), specularMaps.end());

		// If diffuse maps and specular maps, assign first one to material
		//TODO: Find a way to handle forward rendering with multiple diffuse/specular maps per mesh
		if (diffuseMaps.size() != 0) { materialInfo.diffuseMap = diffuseMaps[0]; }
		if (specularMaps.size() != 0) { materialInfo.specularMap = specularMaps[0]; }

		// Load additional material data
		aiColor3D ambientColor; material->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor); materialInfo.ambientColor = glm::vec4(ambientColor.r, ambientColor.g, ambientColor.b, 1.f);
		aiColor3D diffuseColor; material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor); materialInfo.diffuseColor = glm::vec4(diffuseColor.r, diffuseColor.g, diffuseColor.b, 1.f);
		aiColor3D specularColor; material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor); materialInfo.specular = glm::vec4(specularColor.r, specularColor.g, specularColor.b, 1.f);
		float shininess; material->Get(AI_MATKEY_SHININESS, shininess); materialInfo.shininessCoefficient = shininess;
	}

	// Construct and return mesh object
	return new Mesh(readVertices, new VertexFormat(readIndices), new Transform(), materialInfo);
}

/**
*	@brief Process all textures of a certain within a given mesh material and return as texture objects.
*	NOTE: If a texture has already been loaded then it will be assigned via the loaded textures instead of a new TextureWrapper being created for it.
*	@param a_meshMaterial is the mesh material to process textures from.
*	@param a_textureType defines what type of textures to look for e.g. aiTextureType_DIFFUSE for diffuse maps
*	@param a_typeName is the type of texture to set the returned object to.
*	@return vector of processed textures in the form of created TextureWrappers.
*/
std::vector<TextureWrapper*> Model::ReadMaterialTextures(aiMaterial * a_meshMaterial, int a_textureType, std::string a_typeName)
{
	std::vector<TextureWrapper*> processedTextures;

	// Process textures of specified type in materials
	for (unsigned int i = 0; i < a_meshMaterial->GetTextureCount(aiTextureType(a_textureType)); ++i) {
		
		// Get texture path from current texture and convert it to standard string
		aiString str; a_meshMaterial->GetTexture(aiTextureType(a_textureType), i, &str);
		std::string fileName = str.C_Str();

		// Check if texture has already been loaded
		bool skipLoad = false;

		for (unsigned int j = 0; i < m_loadedTextures.size(); ++i) {

			if (fileName == m_loadedTextures[i]->GetFileName()) {	// Texture has already been loaded
				
				// Add already loaded texture, mark texture loading to be skipped, and break out of loop
				processedTextures.push_back(m_loadedTextures[i]); skipLoad = true; break;
			}
		}

		if (!skipLoad) {
			TextureWrapper* newTex = new TextureWrapper((m_modelDirectory + '/' + fileName).c_str(), a_typeName, FILTERING_MIPMAP);	// Backslash must be appended because directory does not have a trailing backslash

			processedTextures.push_back(newTex);

			// Add to loaded textures as well so this texture isn't loaded unecessarily again
			m_loadedTextures.push_back(newTex);
		}
	}

	return processedTextures;
}
