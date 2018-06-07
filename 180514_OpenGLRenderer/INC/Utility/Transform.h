#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace SPRON {
	class Transform {
	public:
		Transform(Transform* a_parentTransform = nullptr,
			const glm::vec3& a_pos = glm::vec3(0), const glm::vec3& a_scale = glm::vec3(1), const glm::vec3& a_rot = glm::vec3(0));
		~Transform();

		const glm::vec3& GetPosition();
		void SetPosition(const glm::vec3& a_pos);

		const glm::vec3& GetScale();
		void SetScale(const glm::vec3& a_scale);

		const glm::vec3& GetRotation();
		void SetRotation(const glm::vec3& a_rotation);

		const glm::mat4& GetMatrix();
		const glm::mat4 GetGlobalMatrix();

		glm::vec3 Forward();
		glm::vec3 Up();
		glm::vec3 Left();

		void Translate(const glm::vec3& a_vec);
	protected:
	private:
		void ReconstructMatrix();
		const glm::mat4& RecurCalculateGlobalMatrix();

		// Cached values (avoids decomposition)
		glm::vec3 m_position;
		glm::vec3 m_scale;
		glm::vec3 m_rotation;

		glm::vec3 m_forward;
		glm::vec3 m_up;
		glm::vec3 m_left;

		glm::mat4 m_transformMatrix;		// Final constructed matrix with position, rotation and scale information

		Transform* m_parentTransform;		// Hold onto parent to ensure that all transformations applied to the parent are also applied to the children
	};
}