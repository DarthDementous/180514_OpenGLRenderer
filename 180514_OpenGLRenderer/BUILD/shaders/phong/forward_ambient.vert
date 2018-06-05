#version 440 core
// NOTE: Ambient pass only requires a position and texture coordinate
layout (location = 0)	in vec4 a_pos;		
layout (location = 1)	in vec2 a_texCoord;

uniform mat4 modelTransform;		// Global space 
uniform mat4 viewTransform;			// View space
uniform mat4 projectionTransform;	// Clip space

varying vec2 vertTexCoord;			// Use varying instead of out so the fragment shader receives an interpolated version of this

void main() {
	// Clip space <- view space <- global space <- local space 
	gl_Position = projectionTransform * viewTransform * modelTransform * a_pos;	// Convert vertice position from local space to Normalized Device Coordinate space
	
	// Output
	vertTexCoord = a_texCoord;
}