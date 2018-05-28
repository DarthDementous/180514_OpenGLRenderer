#include "Transform.h"
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
	m_position = a_pos;				// Store value to avoid needing to decompose matrix
}

const glm::vec3 & Transform::GetScale()
{
	return m_scale;
}

void Transform::SetScale(const glm::vec3 & a_scale)
{
	m_scale = a_scale;				// Store value to avoid needing to decompose matrix
}

const glm::vec3 & Transform::GetRotation()
{
	return m_rotation;
}

void Transform::SetRotation(const glm::vec3 & a_rotation)
{
	m_rotation = a_rotation;		// Store value to avoid needing to decompose matrix
}

const glm::mat4 & Transform::GetMatrix()
{
	ReconstructMatrix();
	return m_transformMatrix;
}

glm::vec3 Transform::Forward()
{
	return m_forward;
}

glm::vec3 Transform::Up()
{
	return m_up;
}

glm::vec3 Transform::Left()
{
	return m_left;
}

void Transform::Translate(const glm::vec3 & a_vec)
{
	m_position += a_vec;

	ReconstructMatrix();
}

/**
*	@brief Combine position, rotation and scale information to construct final matrix.
*	NOTE: Only needs to be called upon retrieving the Matrix because it is not able to be modified otherwise.
*	@return void.
*/
void Transform::ReconstructMatrix() {
	// Reconstruct transform matrix in order of scale, rotate, translate to avoid skewing
	m_transformMatrix = glm::translate(m_position) * 
		//glm::rotate(m_rotation.x, glm::vec3(1, 0, 0)) * glm::rotate(m_rotation.y, glm::vec3(0, 1, 0)) * glm::rotate(m_rotation.z, glm::vec3(0, 0, 1)) *
		glm::mat4_cast(glm::quat(m_rotation)) *		// Convert from quat to mat in order to be compatable in composition operation
		glm::scale(m_scale);

	/// Store transform matrix axis directions
	m_forward	= glm::vec3(m_transformMatrix[2][0], m_transformMatrix[2][1], m_transformMatrix[2][2]);
	m_up		= glm::vec3(m_transformMatrix[1][0], m_transformMatrix[1][1], m_transformMatrix[1][2]);
	m_left		= glm::vec3(m_transformMatrix[0][0], m_transformMatrix[0][1], m_transformMatrix[0][2]);
}
