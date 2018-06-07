#version 440
varying vec2 texCoords;

uniform sampler2D screenRenderTex;
uniform float clarityFactor;		// Determines how un-distorted the final image is, greater values = more clear, smaller values = less clear

void main() {
	float texelOffset = 1.f / clarityFactor;	// The distance between texel neighbor sampling, the smaller the divide (aka the greater the distance) 
												// the greater the distortion (if 1 then its a 1 to 1 sampling of 3x3 neighboring texels i.e. regular filtering)

	/// 3x3 Kernel Sampling
	vec4 texelNeighborSamples[9] = vec4[] (
		texture(screenRenderTex, vec2(texCoords + vec2(-texelOffset, texelOffset))),		// Top left
		texture(screenRenderTex, vec2(texCoords + vec2(0.f, texelOffset))),					// Top middle
		texture(screenRenderTex, vec2(texCoords + vec2(texelOffset, texelOffset))),			// Top right
		
		texture(screenRenderTex, vec2(texCoords + vec2(-texelOffset, 0))),					// Center left
		texture(screenRenderTex, vec2(texCoords + vec2(0.f, 0))),							// Center middle
		texture(screenRenderTex, vec2(texCoords + vec2(texelOffset, 0))),					// Center right

		texture(screenRenderTex, vec2(texCoords + vec2(-texelOffset, -texelOffset))),		// Bottom left
		texture(screenRenderTex, vec2(texCoords + vec2(0.f, -texelOffset))),				// Bottom middle
		texture(screenRenderTex, vec2(texCoords + vec2(texelOffset, -texelOffset)))			// Bottom right
	);

	float sampleKernel [9] = float [] (		// Multiplication values for 'edge detection' effect
		1, 1, 1,
		1, -8, 1,		// Center texel is drastically reduced in color to emphasize its edges
		1, 1, 1
	);

	// Calculate final frag color by getting sum of neighbor texels multiplied by the sample kernel
	vec4 texelSum;

	for (int i = 0; i < 9; ++i) {
		texelSum += texelNeighborSamples[i] * sampleKernel[i];
	}

	gl_FragColor = texelSum;
}