#version 440
varying vec2 texCoords;

uniform sampler2D screenRenderTex;

void main() {
	// Invert color of screen render texel
	gl_FragColor = vec4(vec3(1.0 - texture(screenRenderTex, texCoords)), 1.0);		// Ensure that the w component is still 1 after minusing
}