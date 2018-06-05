#version 440 core
// NOTE: Vertice positions come in as local space (origin)
layout (location = 0)	in vec2 a_screenCoord;		// location = 0 means it represents the position vertex attribute as 0
layout (location = 1)	in vec2 a_texCoord;

uniform int viewportWidth;
uniform int viewportHeight;
uniform int viewportX;
uniform int viewportY;

out vec2 vertTexCoord;

void main() {
	// Convert from screen space to NDC
	float Xndc = (a_screenCoord.x - viewportX) / (viewportWidth / 2) - 1;
	float Yndc = (a_screenCoord.y - viewportY) / (viewportHeight / 2) - 1;

	gl_Position = vec4(Xndc, Yndc, 0, 1);	
}