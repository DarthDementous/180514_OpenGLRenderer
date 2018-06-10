#pragma once

#include <vector>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <string>

namespace SPRON {
	class TextureWrapperBase;
	class PhongLight;
	class PhongLight_Dir;
	class PhongLight_Spot;
	class PhongLight_Point;

	struct Material;
}

namespace SPRON {
	class ShaderWrapper {
	public:
		ShaderWrapper(const std::string& a_name = "");
		~ShaderWrapper();

		void LoadShader(const char* a_filePath, unsigned int a_shaderType, const char* a_headerStr = nullptr);
		void LinkShaders();

		void SetBool(const char* a_name, bool a_val);
		void SetInt(const char* a_name, int a_val);
		void SetFloat(const char* a_name, float a_val);
		void SetVec3(const char* a_name, const glm::vec3& a_val);
		void SetVec4(const char* a_name, const glm::vec4& a_val);
		void SetTexture(const char* a_name, TextureWrapperBase* a_tex);
		void SetMat3(const char* a_name, const glm::mat3& a_val);
		void SetMat4(const char* a_name, const glm::mat4& a_val);
		void SetMaterial(const char* a_name, Material a_mat);
		void SetBaseLight(const char* a_name, PhongLight* a_light);
		void SetDirectionalLight(const char* a_name, PhongLight_Dir* a_light);
		void SetSpotLight(const char* a_name, PhongLight_Spot* a_light);
		void SetPointLight(const char* a_name, PhongLight_Point* a_light);

		std::string GetName() { return m_name; }

		int FindLocation(const char* a_name);

		operator unsigned int() { return m_ID; }	// Allow class to be used in parameters of openGL functions
	protected:
	private:
		unsigned int m_ID;							// OpenGL identifier
		std::string m_name;

		std::vector<unsigned int>	m_shaderIDs;	// List of attached shader IDs
	};
}