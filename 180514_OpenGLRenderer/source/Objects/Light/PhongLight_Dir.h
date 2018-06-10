#pragma once

#include "PhongLight.h"

#include <glm/vec4.hpp>
#include <glm/vec3.hpp>

namespace SPRON {
	class PhongLight_Dir : public PhongLight {
	public:
		PhongLight_Dir(const glm::vec4& a_ambient = glm::vec4(1), const glm::vec4& a_diffuse = glm::vec4(1), const glm::vec4& a_specular = glm::vec4(1),
			const glm::vec3& a_castDir = glm::vec3());
		virtual ~PhongLight_Dir();

		const glm::vec3& GetCastDir();

		void ListenIMGUI(int a_id) override;
	protected:
	private:
		glm::vec3 m_castDir;
	};
}