#pragma once

#include "PhongLight.h"

#include <glm/vec4.hpp>

class PhongLight_Dir : public PhongLight {
public:
	PhongLight_Dir(const glm::vec4& a_ambient = glm::vec4(1), const glm::vec4& a_diffuse = glm::vec4(1), const glm::vec4& a_specular = glm::vec4(1),
		const glm::vec4& a_castDir = glm::vec4(0));
	virtual ~PhongLight_Dir();

	glm::vec4 GetCastDir();
protected:
private:
	glm::vec4 m_castDir;
};