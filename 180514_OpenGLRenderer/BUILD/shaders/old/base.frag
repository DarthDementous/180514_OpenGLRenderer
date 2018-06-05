#version 440 core
out vec4 fragColor;

in vec4 vertColor;					// Setting input with same name as output from another shader allows data to be shared
in vec4 vertPos;
in vec2 vertTexCoord;

uniform sampler2D textureSample0;	// If not manually set, corresponds to texture unit '0' which is activated by default

void main() {
	// Get color from sampled texel
	fragColor = texture(textureSample0, vertTexCoord);
}