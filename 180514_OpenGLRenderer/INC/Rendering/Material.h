#pragma once

#include <vector>

class Material {
public:
	Material();
	~Material();

	void LoadShader(const char* a_filePath, unsigned int a_shaderType);
	void LinkShaders();

	operator unsigned int() { return m_ID; }	// Allow class to be used in parameters of openGL functions
protected:
private:
	unsigned int m_ID;						// OpenGL identifier

	std::vector<unsigned int> m_shaderIDs;	// List of attached shader IDs
};