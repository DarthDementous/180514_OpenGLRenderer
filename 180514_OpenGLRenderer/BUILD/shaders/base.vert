#version 440 core
layout (location = 0)	in vec4 a_pos;		// location = 0 means it represents the position vertex attribute as 0
layout (location = 1)	in vec4 a_color;
layout (location = 2)	in vec2 a_texCoord;

uniform float xOffset;
uniform float yOffset;

out vec4 vertColor;			// Outputs to fragment shader
out vec4 vertPos;
out vec2 vertTexCoord;

void main() {
	gl_Position = glm::vec4(a_pos.x + xOffset, a_pos.y + yOffset, a_pos.z, a_pos.w);	// Set NDC position of vertex based on input (NOTE: gl_Position is a vec4 and requires conversion where w must be 1 in order to be translated)
	
	vertColor		= a_color;
	vertPos			= gl_Position;
	vertTexCoord	= a_texCoord;
}