#version 440 core
out vec4 fragColor;

in vec4 vertColor;					// Setting input with same name as output from another shader allows data to be shared
in vec4 vertPos;
in vec2 vertTexCoord;

uniform float colorMix;

uniform sampler2D textureSample0;	// If not manually set, corresponds to texture unit '0' which is activated by default
uniform sampler2D textureSample1;

void main() {
	// Flip image
	fragColor = mix(texture(textureSample0, vec2(1 - vertTexCoord.x, vertTexCoord.y)),		// Flip first image
	texture(textureSample1, vertTexCoord), colorMix);		// Get filtered color from interpolated texture samples, corresponding with texture coordinate on the vertex

	// Multiply color ontop of texture
	fragColor *= vertColor;
}