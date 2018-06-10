#version 440 core
varying vec2 texCoords;

uniform sampler2D screenRenderTex;
uniform bool enableHDR;
uniform float exposure;			// High = more detail in darker areas (night time), Low = more detail in brighter areas (daylight)

void main() {
	// Sample screen render texel to frag color
	vec4 sampleHDR = texture(screenRenderTex, texCoords);

	vec3 LDRColor = sampleHDR.rgb;		// Set to default un-converted HDR
	if (enableHDR) {
		// Use exposure tone mapping to properly convert HDR color values to LDR without loss of detail and with control over exposure
		LDRColor = vec3(1.0) - exp(-sampleHDR.rgb * exposure);
	}

	// Apply gamma correction
	float gamma = 1.4;			// Standard gamma value, matching power law sensitivity of human vision 
								//(e.g. RGB of (128, 128, 128) will not emit 50% light energy of its pixels but instead 22%.
								//Human vision is non-liner and thus the light source would have to be attenuated to 22% of its original intensity
								//to appear half as bright to humans.
	LDRColor = pow(LDRColor, vec3(1.0 / gamma));

	gl_FragColor = vec4(LDRColor, 1.0);
}