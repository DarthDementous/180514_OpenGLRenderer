#pragma once

#include <vector>
#include <string>
#include <glm/vec4.hpp>

namespace SPRON {
	class Mesh;
	class Texture;
	class RenderCamera;
	class PhongLight;
	class ShaderWrapper;
	class Transform;
}

struct aiNode;
struct aiMesh;
struct aiScene;
struct aiMaterial;

namespace SPRON {
	class Model {
	public:
		Model(const char* a_filePath);
		~Model();

		void Draw(RenderCamera* a_camera,
			std::vector<PhongLight*> a_lights,
			const glm::vec4& a_globalAmbient, ShaderWrapper* a_ambientPass,
			ShaderWrapper* a_directionalPass, ShaderWrapper* a_pointPass, ShaderWrapper* a_spotPass, ShaderWrapper* a_debugPass);

		Transform* GetTransform();

		std::vector<Mesh*> GetModelMeshes();
		std::string GetDirectory();
	protected:
	private:
		Transform * m_modelTransform;						// Global transform for the model, any changes to it apply to all the child meshes' transforms

		std::vector<Texture*> m_loadedTextures;		// Hold onto loaded textures to avoid creating new ones for the same texture files
		std::vector<Mesh*> m_meshes;

		std::string m_modelDirectory;	// Hold onto directory model was loaded in for loading additional files e.g. textures

		/// Model loading functions
		void LoadModel(std::string a_filePath);
		void RecurReadNode(aiNode* a_node, const aiScene* a_modelScene);
		Mesh* ReadMesh(aiMesh* a_mesh, const aiScene* a_modelScene);
		std::vector<Texture*> ReadMaterialTextures(aiMaterial* a_meshMaterial, int a_textureType, std::string a_typeName);
	};
}