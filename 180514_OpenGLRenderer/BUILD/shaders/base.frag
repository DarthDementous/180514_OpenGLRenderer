#version 440 core
out vec4 fragColor;

in vec4 vertColor;				// Setting input with same name as output from another shader allows data to be shared

void main() {
	fragColor = vertColor;	
}