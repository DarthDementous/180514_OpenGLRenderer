#version 440 core
varying vec2 texCoords;

uniform sampler2D screenRenderTex;

void main() {
	// Apply screen render texel to frag color
	gl_FragColor = texture(screenRenderTex, texCoords);

	// Apply gamma correction
	float gamma = 2.2;			// Standard gamma value, matching power law sensitivity of human vision 
								//(e.g. RGB of (128, 128, 128) will not emit 50% light energy of its pixels but instead 22%.
								//Human vision is non-liner and thus the light source would have to be attenuated to 22% of its original intensity
								//to appear half as bright to humans.

	gl_FragColor.rgb = pow(gl_FragColor.rgb, vec3(gamma));
}