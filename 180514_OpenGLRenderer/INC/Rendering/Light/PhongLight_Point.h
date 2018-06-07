#pragma once

#include "PhongLight.h"

#include <glm/vec4.hpp>

namespace SPRON {
	class PhongLight_Point : public PhongLight {
	public:
		PhongLight_Point(const glm::vec4& a_ambient = glm::vec4(1), const glm::vec4& a_diffuse = glm::vec4(1), const glm::vec4& a_specular = glm::vec4(1),
			const glm::vec4& a_pos = glm::vec4(0.f, 0.f, 0.f, 1.f), float a_illuminationRadius = 0.f, float a_minIllumination = 0.f);
		virtual ~PhongLight_Point();

		glm::vec4 GetPos();
		float GetIlluminationRadius();
		float GetMinIllumination();

		void SetPos(const glm::vec4& a_pos);

		// IMGUI getters and setters
		void SetIlluminationRadius(float a_range) { m_illuminationRadius = a_range; }
	protected:
	private:
		glm::vec4 m_pos;

		float m_illuminationRadius;		// Coverage distance of the light
		float m_minIllumination;		// Minimum illumination value before it is zeroed out (like an Epsilon)
	};
}