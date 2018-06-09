
// NOTE: Version, functions and members are defined in header file

uniform GPU_Spot_Light spotLight;

void main() {
	vec4 diffuseSample;
	vec4 specularSample;
	vec3 normalSample;
	vec3 dirToViewer;

	SetLightingParameters(diffuseSample, specularSample, normalSample, dirToViewer);

	// Apply spot lighting pass
	gl_FragColor = CalculateSpotLighting( spotLight, normalSample, dirToViewer, diffuseSample, specularSample);
}