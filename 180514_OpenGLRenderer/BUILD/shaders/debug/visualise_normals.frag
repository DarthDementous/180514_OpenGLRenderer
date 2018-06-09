#version 440 core

// Color from geometry shader
in vec4 outputColor;

void main() {
	gl_FragColor = outputColor;
}