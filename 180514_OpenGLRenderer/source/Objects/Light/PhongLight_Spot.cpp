#include "Light\PhongLight_Spot.h"

#include <glm/ext.hpp>

namespace SPRON {

	PhongLight_Spot::PhongLight_Spot(const glm::vec4 & a_ambient, const glm::vec4 & a_diffuse, const glm::vec4 & a_specular,
		const glm::vec4 & a_pos, const glm::vec3 & a_dir, float a_spotInnerAngle, float a_spotOuterAngle) :
		PhongLight(a_ambient, a_diffuse, a_specular), m_pos(a_pos), m_spotDir(a_dir)
	{
		m_type = SPOT_LIGHT;

		// Get cosines of passed in angles
		m_spotInnerCosine = glm::cos(glm::radians(a_spotInnerAngle));
		m_spotOuterCosine = glm::cos(glm::radians(a_spotOuterAngle));
	}
	PhongLight_Spot::~PhongLight_Spot()
	{
	}

	glm::vec4 PhongLight_Spot::GetPos()
	{
		return m_pos;
	}

	glm::vec3 PhongLight_Spot::GetSpotDir()
	{
		return m_spotDir;
	}

	float PhongLight_Spot::GetSpotInnerCosine()
	{
		return m_spotInnerCosine;
	}

	float PhongLight_Spot::GetSpotOuterCosine()
	{
		return m_spotOuterCosine;
	}


	void PhongLight_Spot::SetPos(const glm::vec4 & a_pos)
	{
		m_pos = a_pos;
	}

	void PhongLight_Spot::SetSpotDir(const glm::vec3 & a_dir)
	{
		m_spotDir = a_dir;
	}
}
