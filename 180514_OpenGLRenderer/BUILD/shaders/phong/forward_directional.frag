
// NOTE: Version, functions and members are defined in header file

uniform GPU_Dir_Light dirLight;

void main() {
	vec4 diffuseSample;
	vec4 specularSample;
	vec3 normalSample;
	vec3 dirToViewer;

	SetLightingParameters(diffuseSample, specularSample, normalSample, dirToViewer);

	// Apply directional lighting pass
	gl_FragColor = CalculateRawLighting( dirLight.base, -dirLight.castDir, normalSample, dirToViewer, diffuseSample, specularSample);
}