#version 440 core
layout (location = 0) in vec2 a_pos;					// 2D NDC coordinates
layout (location = 1) in vec2 a_texCoords;

varying vec2 texCoords;

void main() {
	gl_Position = vec4(a_pos.x, a_pos.y, 0.f, 1.f);
	
	texCoords = a_texCoords;
}