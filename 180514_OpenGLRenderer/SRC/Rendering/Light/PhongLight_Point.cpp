#include "Light\PhongLight_Point.h"

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
