#version 440 core
varying vec2 vertTexCoord;		// Take in interpolated version of tex coordinate passed in by vert shader

uniform vec4		ambient;			// Contains light color and intensity
uniform sampler2D	texSample;
uniform bool		useTex;				// Whether texture is valid to use or not
uniform bool		correctGamma;		// Whether to apply gamma correction or not

void main() {
	// Ambient lighting pass on fragment
	vec4 finalAmbient = vec4(0.6f, 0.2f, 0.7f, 1.f);	// Set to default 'texture not found' color
	if (useTex == true) { finalAmbient = ambient * texture(texSample, vertTexCoord); }

	// Apply gamma correction if enabled
	if (correctGamma) { 
		float gamma = 2.2;
		finalAmbient.rgb = pow(finalAmbient.rgb, vec3(1.0/gamma));
	}

	gl_FragColor = finalAmbient;
}