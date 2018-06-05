#pragma once

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

// Holds onto data contained within the vertex for readable usage with openGL code
struct Vertex {
	Vertex(const glm::vec4& a_pos, const glm::vec2& a_coord, const glm::vec4& a_normal) {
		pos = a_pos;
		texCoord = a_coord;
		normal = a_normal;
	}

	glm::vec4 pos;
	glm::vec2 texCoord;
	glm::vec4 normal;
};