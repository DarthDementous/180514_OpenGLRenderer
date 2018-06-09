#pragma once

#include "Vertex.h"

#include <vector>
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>
#include <imgui.h>

namespace SPRON {
	class RenderCamera;

	class VertexFormat;
	class ShaderWrapper;
	class Texture;
	class Transform;
	class PhongLight;
}

namespace SPRON {
	#pragma region Structs
	// Holds onto information to be passed into the forward rendering light shader programs
	struct Material {
	public:
		Material(const glm::vec4& a_ambient = glm::vec4(1.f),
			const glm::vec4& a_diffuse = glm::vec4(1.f),
			const glm::vec4& a_specular = glm::vec4(1.f),
			float a_shininess = 32.f, Texture* a_diffuseMap = nullptr, Texture* a_specularMap = nullptr, Texture* a_normalMap = nullptr,
			const std::string& a_name = "") {

			static int currID = 0;
			m_id = currID++;

			ambientColor = a_ambient;
			diffuseColor = a_diffuse;
			specular = a_specular;

			shininessCoefficient = a_shininess;

			diffuseMap = a_diffuseMap;
			specularMap = a_specularMap;
			normalMap = a_normalMap;

			name = a_name;
		}
		~Material() {}

		/// IMGUI
		void ListenIMGUI() {
			ImGui::LabelText("", name.c_str());
			ImGui::Checkbox("Disable Diffuse Mapping", &disableDiffuseMap);
			ImGui::Checkbox("Disable Specular Mapping", &disableSpecularMap);
			ImGui::Checkbox("Disable Normal Mapping", &disableNormalMap);
			ImGui::NewLine();
		}

		std::string name;

		glm::vec4 ambientColor;
		glm::vec4 diffuseColor;
		glm::vec4 specular;

		float shininessCoefficient;

		Texture* diffuseMap = nullptr;		// 'Texture' of the object
		bool disableDiffuseMap = false;

		Texture* specularMap = nullptr;
		bool disableSpecularMap = false;

		Texture* normalMap = nullptr;
		bool disableNormalMap = false;

	private:
		int m_id;
	};

#pragma endregion 

	class Mesh {
	public:
		Mesh() {}
		Mesh(const std::vector<Vertex>& a_verts, VertexFormat* a_format, Transform* a_transform,
			Material a_material = Material());		// Set default material values if not defined in constructor
		~Mesh();

		Material& GetMaterial();
		Transform* GetTransform();
		std::vector<Vertex> GetVerticeData() { return m_rawVerticeData; }

		operator unsigned int() { return m_vertBufferID; }

		void Draw(RenderCamera* a_camera,
			std::vector<PhongLight*> a_lights,
			const glm::vec4& a_globalAmbient, ShaderWrapper* a_ambientPass,
			ShaderWrapper* a_directionalPass, ShaderWrapper* a_pointPass, ShaderWrapper* a_spotPass, ShaderWrapper* a_debugPass);

		void SetMaterial(Material a_material);

		void Render(ShaderWrapper* a_shaderProgram);
	protected:
	private:
		unsigned int m_vertBufferID;	// Hold onto vertex buffer identifier

		Material m_material;

		VertexFormat* m_vertFormat;			// How vertex data is interpreted

		std::vector<Vertex> m_rawVerticeData;	// Keep track of vertex data so memory isn't freed until Mesh is deleted

		Transform* m_parentTransform;	// Hold onto parent transform so that changes made to it will apply to all of its child meshes
		Transform* m_transform;			// Transform information in global space
	};
}