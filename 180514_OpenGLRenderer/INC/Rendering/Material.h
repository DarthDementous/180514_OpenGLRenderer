#pragma once

#include <vector>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

class TextureWrapper;

class Material {
public:
	Material();
	~Material();

	void LoadShader(const char* a_filePath, unsigned int a_shaderType);
	void LinkShaders();

	void SetBool(const char* a_name, bool a_val);
	void SetInt(const char* a_name, int a_val);
	void SetFloat(const char* a_name, float a_val);
	void SetVec3(const char* a_name, const glm::vec3& a_val);
	void SetVec4(const char* a_name, const glm::vec4& a_val);
	void SetTexture(const char* a_name, TextureWrapper* a_tex);
	void SetMat4(const char* a_name, const glm::mat4& a_val);

	operator unsigned int() { return m_ID; }	// Allow class to be used in parameters of openGL functions
protected:
private:
	unsigned int m_ID;						// OpenGL identifier

	std::vector<unsigned int>	m_shaderIDs;	// List of attached shader IDs
};