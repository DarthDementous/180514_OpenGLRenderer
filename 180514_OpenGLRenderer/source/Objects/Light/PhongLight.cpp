#include "Light\PhongLight.h"

#include <imgui.h>

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

	void PhongLight::ListenIMGUI(int a_id)
	{
		// Color properties
		ImGui::ColorEdit4("Ambient", &m_ambient[0], ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
		ImGui::ColorEdit4("Diffuse", &m_diffuse[0], ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
		ImGui::ColorEdit4("Specular", &m_specular[0], ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
	}
}