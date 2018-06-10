#include "Light\PhongLight_Dir.h"

#include <imgui.h>

namespace SPRON {

	PhongLight_Dir::PhongLight_Dir(const glm::vec4 & a_ambient, const glm::vec4 & a_diffuse, const glm::vec4 & a_specular, const glm::vec3 & a_castDir) :
		PhongLight(a_ambient, a_diffuse, a_specular), m_castDir(a_castDir)
	{
		m_type = DIRECTIONAL_LIGHT;
	}

	PhongLight_Dir::~PhongLight_Dir()
	{
	}

	const glm::vec3& PhongLight_Dir::GetCastDir()
	{
		return m_castDir;
	}
	void PhongLight_Dir::ListenIMGUI(int a_id)
	{
		ImGui::LabelText("", "Directional Light %i", a_id);

		PhongLight::ListenIMGUI(a_id);

		// Directional light properties
		ImGui::DragFloat3("Cast Direction", &m_castDir[0], 0.01f, -1, 1);
	}
}