#pragma once

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>

namespace SPRON {
	// Holds onto data contained within the vertex for readable usage with openGL code
	struct Vertex {
		Vertex(const glm::vec4& a_pos, const glm::vec2& a_coord = glm::vec2(), 
			const glm::vec3& a_normal = glm::vec3(), const glm::vec4& a_tangent = glm::vec4()) {

			pos = a_pos;
			texCoord = a_coord;
			normal = a_normal;
			normalTangent = a_tangent;
		}

		glm::vec4 pos;
		glm::vec2 texCoord;
		glm::vec3 normal;
		glm::vec4 normalTangent;		// Right direction to the normal, stores handedness of bitanget in its w component
	};
}