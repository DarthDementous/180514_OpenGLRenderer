#include "Light\PhongLight.h"

namespace SPRON {

	PhongLight::PhongLight(const glm::vec4& a_ambient, const glm::vec4& a_diffuse, const glm::vec4& a_specular) :
		m_ambient(a_ambient), m_diffuse(a_diffuse), m_specular(a_specular)
	{
	}

	PhongLight::~PhongLight()
	{
	}

	eLightType PhongLight::GetType()
	{
		return m_type;
	}

	glm::vec4 PhongLight::GetAmbient()
	{
		return m_ambient;
	}

	glm::vec4 PhongLight::GetDiffuse()
	{
		return m_diffuse;
	}

	glm::vec4 PhongLight::GetSpecular()
	{
		return m_specular;
	}
}