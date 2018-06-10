#include "Light\PhongLight_Point.h"

#include <imgui.h>
#include <iostream>
#include <glm/vec3.hpp>

namespace SPRON {

	PhongLight_Point::PhongLight_Point(const glm::vec4 & a_ambient, const glm::vec4 & a_diffuse, const glm::vec4 & a_specular,
		const glm::vec4 & a_pos, float a_illuminationRadius, float a_minIllumination) :
		PhongLight(a_ambient, a_diffuse, a_specular), m_pos(a_pos), m_illuminationRadius(a_illuminationRadius), m_minIllumination(a_minIllumination)
	{
		m_type = POINT_LIGHT;
	}

	PhongLight_Point::~PhongLight_Point()
	{
	}

	glm::vec4 PhongLight_Point::GetPos()
	{
		return m_pos;
	}

	float PhongLight_Point::GetIlluminationRadius()
	{
		return m_illuminationRadius;
	}

	float PhongLight_Point::GetMinIllumination()
	{
		return m_minIllumination;
	}

	void PhongLight_Point::SetPos(const glm::vec4 & a_pos)
	{
		m_pos = a_pos;
	}

	/**
	*	@brief Call from update to be able to modify properties of point light.
	*/
	void PhongLight_Point::ListenIMGUI(int a_id)
	{
		ImGui::LabelText("", "Point Light %i", a_id);

		PhongLight::ListenIMGUI(a_id);		// Call base imgui property displayer
		
		// Point light properties
		ImGui::DragFloat3("Position", &m_pos[0], 0.25f);
		ImGui::DragFloat("Illumination Radius", &m_illuminationRadius, 0.5f, 0.f, 200.f);
		ImGui::DragFloat("Minimum Illumination", &m_minIllumination, 0.001, 0.f, 1.f);

	}
}