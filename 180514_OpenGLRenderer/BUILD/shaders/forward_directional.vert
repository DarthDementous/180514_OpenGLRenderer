#version 440 core

layout (location = 0)	in vec4 a_pos;		
layout (location = 1)	in vec2 a_texCoord;
layout (location = 2)	in vec4 a_normal;

// Use varying instead of out so the fragment shader receives an interpolated version of this
varying vec2 vertTexCoord;			
varying vec4 vertNormal;
varying vec4 fragPos;				// World position of fragment

uniform mat4 modelTransform;		// Global space 
uniform mat4 viewTransform;			// View space
uniform mat4 projectionTransform;	// Clip space

void main() {
	// Clip space <- view space <- global space <- local space 
	gl_Position = projectionTransform * viewTransform * modelTransform * a_pos;	// Convert vertice position from local space to Normalized Device Coordinate space
	
	// Output
	vertTexCoord = a_texCoord;
	vertNormal	 = mat4(transpose(inverse(modelTransform))) * a_normal;			// Rotate normals according to model transform
	fragPos		 = modelTransform * a_pos;
}