#version 440 core
layout (location = 0) 

in vec3 aPos;

void main() {
	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);		// Set NDC position of vertex based on input (NOTE: gl_Position is a vec4 and requires conversion where w must be 1 in order to be translated)
}