#version 440
varying vec2 texCoords;

uniform sampler2D screenRenderTex;

void main() {
	// Apply screen render texel to frag color
	gl_FragColor = texture(screenRenderTex, texCoords);
}