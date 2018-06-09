#version 440 core

layout (location = 0) in vec4 a_pos;
layout (location = 2) in vec3 a_normal;
layout (location = 3) in vec4 a_normalTangent;

uniform mat4 modelTransform;

// Data to geometry shader
out vec3 worldNormal;
out vec3 worldTangent;
flat out float bitangentHandedness;		// Make sure value doesn't get interpolated and always stays as 1 or -1

void main() {
	// Only output world position so its easier for the geometry shader to calculate debug data
	gl_Position = modelTransform * a_pos;
	
	// Calculate normal, tangent and bitangent in world coordinates
	worldNormal		= normalize(vec3(modelTransform * vec4(a_normal, 0)));				
	worldTangent	= normalize(vec3(modelTransform * vec4(a_normalTangent.xyz, 0)));	// Do not modify w component holding onto handedness	
	bitangentHandedness = a_normalTangent.w;
}