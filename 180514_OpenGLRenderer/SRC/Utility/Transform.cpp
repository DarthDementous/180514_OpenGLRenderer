#include "Transform.h"
#include <glm/ext.hpp>

namespace SPRON {

	Transform::Transform(Transform* a_parentTransform, const glm::vec3& a_pos, const glm::vec3& a_scale, const glm::vec3& a_rot) :
		m_position(a_pos), m_scale(a_scale), m_rotation(a_rot), m_parentTransform(a_parentTransform)
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

		ReconstructMatrix();
	}

	const glm::vec3 & Transform::GetScale()
	{
		return m_scale;
	}

	void Transform::SetScale(const glm::vec3 & a_scale)
	{
		m_scale = a_scale;				// Store value to avoid needing to decompose matrix

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

	/**
	*	@brief Recursively travel through the current transform's parents to determine its global transform matrix.
	*	@return Calculated global transform matrix.
	*/
	const glm::mat4 & Transform::RecurCalculateGlobalMatrix()
	{
		if (m_parentTransform == nullptr) {		// No parent found, return regular transform matrix
			return m_transformMatrix;
		}

		return m_parentTransform->RecurCalculateGlobalMatrix() * m_transformMatrix;		// Calculate global matrix of current transform by timesing its parent's global matrix by its own local transform
	}

	/**
	*	@brief Calculate and return global transform matrix.
	*	@return Global transform matrix for this transform.
	**/
	const glm::mat4 Transform::GetGlobalMatrix()
	{
		if (m_parentTransform == nullptr) {		// No parent found, return local transform
			return m_transformMatrix;
		}

		return m_parentTransform->RecurCalculateGlobalMatrix() * m_transformMatrix;		// Global matrix = global matrix of parent * local matrix
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
			glm::mat4_cast(glm::quat(m_rotation)) *		// Convert from quat to mat in order to be compatable in composition operation
			glm::scale(m_scale);

		// Apply parent transformation if transform has parent
		if (m_parentTransform) { m_transformMatrix = m_parentTransform->RecurCalculateGlobalMatrix() * m_transformMatrix; };

		/// Store transform matrix axis directions
		m_forward = glm::vec3(m_transformMatrix[2][0], m_transformMatrix[2][1], m_transformMatrix[2][2]);
		m_up = glm::vec3(m_transformMatrix[1][0], m_transformMatrix[1][1], m_transformMatrix[1][2]);
		m_left = glm::vec3(m_transformMatrix[0][0], m_transformMatrix[0][1], m_transformMatrix[0][2]);
	}
}