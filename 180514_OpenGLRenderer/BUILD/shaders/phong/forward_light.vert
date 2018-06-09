#version 440 core

layout (location = 0)	in vec4 a_pos;		
layout (location = 1)	in vec2 a_texCoord;
layout (location = 2)	in vec3 a_normal;
layout (location = 3)	in vec4 a_normalTangent;

uniform mat4 modelTransform;		// Global space
uniform mat4 viewTransform;			// View space
uniform mat4 projectionTransform;	// Clip space

// Fragment shader receives an interpolated version of this
//// World space
out vec2 vertTexCoord;			
out vec3 worldNormal;
out vec3 worldTangent;
out vec3 worldFragPos;

flat out float bitangentHandedness;		// Make sure value doesn't get interpolated and always stays as 1 or -1

void main() {
	// Clip space <- view space <- global space <- local space 
	gl_Position = projectionTransform * viewTransform * modelTransform * a_pos;	// Convert vertice position from local space to Normalized Device Coordinate space

	//// Convert all non-world space variables to world space to ensure that lighting operations all happen in the same space
	worldNormal		= normalize(vec3(modelTransform * vec4(a_normal, 0)));				
	worldTangent	= normalize(vec3(modelTransform * vec4(a_normalTangent.xyz, 0)));	// Do not modify w component holding onto handedness	

	worldFragPos = vec3(modelTransform * a_pos);

	//// Output
	vertTexCoord = a_texCoord;
	bitangentHandedness = a_normalTangent.w;			// Extrapolate bitangent handedness
}