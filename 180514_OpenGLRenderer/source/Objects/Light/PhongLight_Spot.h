#pragma once

#include "PhongLight.h"

#include <glm/vec4.hpp>
#include <glm/vec3.hpp>

namespace SPRON {
	class PhongLight_Spot : public PhongLight {
	public:
		PhongLight_Spot(const glm::vec4& a_ambient = glm::vec4(1), const glm::vec4& a_diffuse = glm::vec4(1), const glm::vec4& a_specular = glm::vec4(1),
			const glm::vec4& a_pos = glm::vec4(0.f, 0.f, 0.f, 1.f), const glm::vec3& a_dir = glm::vec3(), float a_spotInnerAngle = 0.f, float a_spotOuterAngle = 0.f);
		virtual ~PhongLight_Spot();

		glm::vec4 GetPos();
		glm::vec3 GetSpotDir();
		float GetSpotInnerCosine();
		float GetSpotOuterCosine();

		void SetPos(const glm::vec4& a_pos);
		void SetSpotDir(const glm::vec3& a_dir);

		void ListenIMGUI(int a_id) override;
	protected:
	private:
		glm::vec4 m_pos;

		glm::vec3	m_spotDir;
		float		m_spotInnerCosine;	// Cosine value of the inner angle of the cone where objects are fully illuminated
		float		m_spotOuterCosine;	// Cosine value of the outer angle of the cone where objects start to interpolate between fully illuminated and not illuminated at all

	};
}