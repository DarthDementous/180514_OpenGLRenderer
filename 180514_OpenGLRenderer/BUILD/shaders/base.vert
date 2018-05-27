#version 440 core
layout (location = 0)		// = 0 means it represents the position vertex attribute as 0
in vec3 a_pos;

void main() {
	gl_Position = vec4(a_pos.x, a_pos.y, a_pos.z, 1.0);		// Set NDC position of vertex based on input (NOTE: gl_Position is a vec4 and requires conversion where w must be 1 in order to be translated)
}