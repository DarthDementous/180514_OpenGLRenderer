
// NOTE: Version, functions and members are defined in header file

uniform GPU_Pt_Light ptLight;

void main() {
	vec4 diffuseSample;
	vec4 specularSample;
	vec3 normalSample;
	vec3 dirToViewer;

	SetLightingParameters(diffuseSample, specularSample, normalSample, dirToViewer);

	// Apply point lighting pass
	gl_FragColor = CalculatePointLighting( ptLight, normalSample, dirToViewer, diffuseSample, specularSample);
}