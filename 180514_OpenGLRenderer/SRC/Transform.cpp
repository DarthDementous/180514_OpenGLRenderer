#include "..\INC\Transform.h"
#include <glm/ext.hpp>

Transform::Transform(const glm::vec3& a_pos, const glm::vec3& a_scale, const glm::vec3& a_rot) : m_position(a_pos), m_scale(a_scale), m_rotation(a_rot)
{
	/// Variable initialisation
	ReconstructMatrix();
}

Transform::~Transform()
{
}

const glm::vec3 & Transform::GetPosition()
{
	return m_position;
}

void Transform::SetPosition(const glm::vec3 & a_pos)
{
	m_position = a_pos;		// Store value to avoid needing to decompose matrix

	ReconstructMatrix();
}

const glm::vec3 & Transform::GetScale()
{
	return m_scale;
}

void Transform::SetScale(const glm::vec3 & a_scale)
{
	m_scale = a_scale;		// Store value to avoid needing to decompose matrix

	ReconstructMatrix();
}

const glm::vec3 & Transform::GetRotation()
{
	return m_rotation;
}

void Transform::SetRotation(const glm::vec3 & a_rotation)
{
	m_rotation = a_rotation;		// Store value to avoid needing to decompose matrix

	ReconstructMatrix();
}

const glm::mat4 & Transform::GetMatrix()
{
	return m_transformMatrix;
}

glm::vec3 Transform::Forward()
{
	return glm::vec3(m_transformMatrix[2][0], m_transformMatrix[2][1], m_transformMatrix[2][2]);
}

glm::vec3 Transform::Up()
{
	return glm::vec3(m_transformMatrix[1][0], m_transformMatrix[1][1], m_transformMatrix[1][2]);
}

glm::vec3 Transform::Left()
{
	return glm::vec3(m_transformMatrix[0][0], m_transformMatrix[0][1], m_transformMatrix[0][2]);
}

void Transform::Translate(const glm::vec3 & a_vec)
{
	m_position += a_vec;

	ReconstructMatrix();
}

void Transform::ReconstructMatrix() {
	// Reconstruct transform matrix in order of scale, rotate, translate to avoid skewing
	m_transformMatrix = glm::translate(m_position) * 
		glm::rotate(m_rotation.x, glm::vec3(1, 0, 0)) * glm::rotate(m_rotation.y, glm::vec3(0, 1, 0)) * glm::rotate(m_rotation.z, glm::vec3(0, 0, 1)) *
		glm::scale(m_scale);
}
