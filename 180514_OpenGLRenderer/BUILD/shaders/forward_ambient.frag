#version 440

varying vec2 vertTexCoord;		// Take in interpolated version of tex coordinate passed in by vert shader

uniform vec4		ambient;			// Contains light color and intensity
uniform sampler2D	texSample;
uniform bool		useTex;				// Whether texture is valid to use or not

void main() {
	// Ambient lighting pass on fragment
	vec4 finalAmbient = vec4(0.6f, 0.2f, 0.7f, 1.f);	// Set to default 'texture not found' color
	if (useTex == true) { finalAmbient = ambient * texture(texSample, vertTexCoord); }

	gl_FragColor = finalAmbient;
}