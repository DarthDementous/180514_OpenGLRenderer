#version 440
varying vec2 texCoords;

uniform sampler2D screenRenderTex;
uniform bool correctGamma;

void main() {
	// Apply screen render texel to frag color
	gl_FragColor = texture(screenRenderTex, texCoords);

	// Apply gamma correction to screen if enabled
	if (correctGamma) { 
		float gamma = 2.2;
		gl_FragColor.rgb = pow(gl_FragColor.rgb, vec3(1.0/gamma));
	}
}