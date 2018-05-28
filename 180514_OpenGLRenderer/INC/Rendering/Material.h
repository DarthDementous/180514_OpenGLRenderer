#pragma once

#include <vector>

class Material {
public:
	Material();
	~Material();

	void LoadShader(const char* a_filePath, unsigned int a_shaderType);
	void LinkShaders();

	void SetBool(const char* a_name, bool a_val);
	void SetInt(const char* a_name, int a_val);
	void SetFloat(const char* a_name, float a_val);

	operator unsigned int() { return m_ID; }	// Allow class to be used in parameters of openGL functions
protected:
private:
	unsigned int m_ID;						// OpenGL identifier

	std::vector<unsigned int> m_shaderIDs;	// List of attached shader IDs
};