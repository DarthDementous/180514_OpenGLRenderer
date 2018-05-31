#version 440 core
// NOTE: Vertice positions come in as local space (origin)
layout (location = 0)	in vec4 a_pos;		// location = 0 means it represents the position vertex attribute as 0
layout (location = 1)	in vec4 a_color;
layout (location = 2)	in vec2 a_texCoord;
layout (location = 3)	in vec4 a_normal;

uniform mat4 modelTransform;		// Global space 
uniform mat4 viewTransform;			// View space
uniform mat4 projectionTransform;	// Clip space
uniform float xOffset;
uniform float yOffset;

out vec4 vertColor;				// Outputs to fragment shader
out vec4 vertPos;
out vec4 vertNormal;
out vec2 vertTexCoord;

out vec4 fragPos;				// Global coordinates of fragment

void main() {
	// Clip space <- view space <- global space <- local space 
	gl_Position = projectionTransform * viewTransform * modelTransform * a_pos;	// Convert vertice position from local space to Normalized Device Coordinate space
	
	vertPos			= gl_Position;
	vertColor		= a_color;
	vertTexCoord	= a_texCoord;
	vertNormal		= mat4(transpose(inverse(modelTransform))) * a_normal;		// Transform normal with modified global transform to get normal properly globally transformed

	fragPos			= modelTransform * a_pos;
}