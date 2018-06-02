#pragma once

#include <glm/vec4.hpp>

class PhongLight {
public:
	PhongLight(const glm::vec4& a_ambient = glm::vec4(1), const glm::vec4& a_diffuse = glm::vec4(1), const glm::vec4& a_specular = glm::vec4(1));
	virtual ~PhongLight() = 0;		// Pure virtual class

	glm::vec4 GetAmbient();
	glm::vec4 GetDiffuse();
	glm::vec4 GetSpecular();
protected:
private:
	glm::vec4 m_ambient;
	glm::vec4 m_diffuse;
	glm::vec4 m_specular;
};