#include "Light\PhongLight_Dir.h"

PhongLight_Dir::PhongLight_Dir(const glm::vec4 & a_ambient, const glm::vec4 & a_diffuse, const glm::vec4 & a_specular, const glm::vec4 & a_castDir) :
	PhongLight(a_ambient, a_diffuse, a_specular), m_castDir(a_castDir)
{
	m_type = DIRECTIONAL_LIGHT;
}

PhongLight_Dir::~PhongLight_Dir()
{
}

glm::vec4 PhongLight_Dir::GetCastDir()
{
	return m_castDir;
}
