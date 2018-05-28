#version 440 core
layout (location = 0)	in vec4 a_pos;		// location = 0 means it represents the position vertex attribute as 0
layout (location = 1)	in vec4 a_color;

out vec4 vertColor;			// Outputs to fragment shader

void main() {
	gl_Position = a_pos;	// Set NDC position of vertex based on input (NOTE: gl_Position is a vec4 and requires conversion where w must be 1 in order to be translated)
	vertColor	= a_color;
}