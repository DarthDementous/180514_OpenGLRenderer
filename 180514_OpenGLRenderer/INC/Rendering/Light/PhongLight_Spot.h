#pragma once

#include "PhongLight.h"

#include <glm/vec4.hpp>

namespace SPRON {
	class PhongLight_Spot : public PhongLight {
	public:
		PhongLight_Spot(const glm::vec4& a_ambient = glm::vec4(1), const glm::vec4& a_diffuse = glm::vec4(1), const glm::vec4& a_specular = glm::vec4(1),
			const glm::vec4& a_pos = glm::vec4(0.f, 0.f, 0.f, 1.f), const glm::vec4& a_dir = glm::vec4(0), float a_spotInnerAngle = 0.f, float a_spotOuterAngle = 0.f);
		virtual ~PhongLight_Spot();

		glm::vec4 GetPos();
		glm::vec4 GetSpotDir();
		float GetSpotInnerCosine();
		float GetSpotOuterCosine();

		void SetPos(const glm::vec4& a_pos);
		void SetSpotDir(const glm::vec4& a_dir);
	protected:
	private:
		glm::vec4 m_pos;

		glm::vec4	m_spotDir;
		float		m_spotInnerCosine;	// Cosine value of the inner angle of the cone where objects are fully illuminated
		float		m_spotOuterCosine;	// Cosine value of the outer angle of the cone where objects start to interpolate between fully illuminated and not illuminated at all

	};
}