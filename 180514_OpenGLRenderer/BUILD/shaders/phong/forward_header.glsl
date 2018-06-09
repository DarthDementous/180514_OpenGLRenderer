#version 440 core

// Struct holding data shared between every type of light
struct GPU_Light_Base {
	// NOTE: Controls color and intensity
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};

// Struct holding data concerned with light attenuation (light fall off based on distance)
struct GPU_Light_Attenuation {
	float illuminationRadius;	// Coverage distance of light
	float minIllumination;		// The minimum illumination of the light before it is cut-off, applied across the illumination radius (like an epsilon)
};

struct GPU_Pt_Light {
	vec4 position;		// Must have a w component of 1
	
	GPU_Light_Attenuation attenuation;
	GPU_Light_Base base;
};

struct GPU_Spot_Light {
	vec4	position;
	vec3	spotDir;						// Direction spotlight is aiming in
	float	spotInnerCosine;				// The cosine of the angle that specifies the radius of the cone in which objects are lit
	float	spotOuterCosine;

	//GPU_Light_Attenuation attenuation;
	GPU_Light_Base base;
};

struct GPU_Dir_Light {
	vec3	castDir;		// Direction light is pointing in
	
	GPU_Light_Base base;
};

struct GPU_Material {		// Serves as a container of uniform variables, accessed via the instance name and then the variable e.g. "material.ambient"
	vec4	ambientColor;
	vec4	diffuseColor;			// Color of object in response to diffuse lighting	(usually same as ambient)
	vec4	specular;				// Color and intensity of object in response to specular lighting (R, G and B values are usually the same)
	float	shininessCoefficient;	// How sharp the curve of the specular highlights are [lower = softer and more spread out, higher = sharper and narrower]

	sampler2D		diffuseMap;		// Texture artistically created with baked in lighting
	bool			useDiffuseMap;	// Check on the CPU if the texture is valid and set this to true or false to determine whether to use it 

	sampler2D	specularMap;	// Texture in a one color spectrum that defines the highlight points for specular
	bool		useSpecularMap;

	sampler2D	normalMap;
	bool		useNormalMap;
};

// VARYING NOTE: Vec4s will be treated like colors! Meaning that the w component will be interpolated, leading to possibly incorrect calculations. 
// Make sure everything comes in as a vec3. The same goes for matrices, unused data will be interpolated.
in vec2 vertTexCoord;			
in float bitangentHandedness;
//// World space
in vec3 worldNormal;
in vec3 worldTangent;
in vec3 worldFragPos;

uniform mat4 modelTransform;
uniform vec3 worldViewerPos;
uniform GPU_Material material;

/**
*	@brief Calculate illumination factor for fragment based on its distance and attenuation data of the light.
*	@param a_dist is the distance from the fragment to the light.
*	@param a_attenuationData is the data concerning how the light falls off.
*	@return illumination factor that can be used to scale the ambient, diffuse and specular light.
*/
float CalculateIllumination(float a_dist, GPU_Light_Attenuation a_attenuationData) {
	float denominator		= a_dist / a_attenuationData.illuminationRadius + 1;
	float illumination		= 1 / (denominator * denominator);
	
	// Apply attenuation rules to illumination
	illumination = (illumination - a_attenuationData.minIllumination) / (1 - a_attenuationData.minIllumination);	// 1. Beyond illumination radius, illumination becomes 0
	illumination = max(illumination, 0);																			// 2. Illumination is at full intensity when distance from light is 0

	return illumination;
}

/*
	@brief Calculate and return color of fragment after applying raw phong lighting (not taking illumination into account)
	@param a_lightBase is the data to get base phong lighting information from.
	@param a_dirToLight is the direction from the fragment to the light.
	@param a_normal is the direction the fragment is pointing in which light will get reflected from.
	@param a_dirToViewer is the direction from the fragment to the viewer.
	@param a_diffuseSample is the color of the sampled texel of the diffuse map for this fragment.
	@param a_specularSample is the color of the sampled texel of the specular map for this fragment.
	@return vec4 containing color information for a fragment lit by raw phong light.
*/
vec4 CalculateRawLighting(GPU_Light_Base a_lightBase, vec3 a_dirToLight, vec3 a_normal, vec3 a_dirToViewer, vec4 a_diffuseSample, vec4 a_specularSample) {
	// Calculate ambient
	vec4 finalAmbient = a_lightBase.ambient * material.ambientColor * a_diffuseSample;

	// Calculate diffuse
	float	diffuseScale	= max(dot(a_normal, a_dirToLight), 0.0f);		// How much fragment is impacted by the light based off its direction (fragment -> light source)
	vec4	finalDiffuse	= a_lightBase.diffuse * diffuseScale * material.diffuseColor * a_diffuseSample;

	// Calculate specular (Blinn-phong model)
	vec3	halfwayDir		= normalize(a_dirToLight + a_dirToViewer);		// Direction halfway in between normal and light direction, avoids negative specular scale if view angle is greater than 90 degrees
	float	specularScale	= pow(max(dot(a_normal, halfwayDir), 0.0), material.shininessCoefficient);
	vec4	finalSpecular	= a_lightBase.specular * specularScale * material.specular * a_specularSample;

	return (finalAmbient + finalDiffuse + finalSpecular);
}

/*
	@brief Calculate and return color of fragment after applying point lighting.
	@param a_lightSource is the point light to get lighting information from.
	@param a_normal is the direction in which light gets reflected from.
	@param a_viewerDir is the direction from the fragment to the viewer's position.
	@param a_diffuseSample is the color of the sampled texel of the diffuse map for this fragment.
	@param a_specularSample is the color of the sampled texel of the specular map for this fragment.
	@return vec4 containing color information for a fragment lit by a point light.
*/
vec4 CalculatePointLighting(GPU_Pt_Light a_lightSource, vec3 a_normal, vec3 a_viewerDir, vec4 a_diffuseSample, vec4 a_specularSample) {
	vec3 dirToLight = normalize(a_lightSource.position.xyz - worldFragPos); 
	
	vec4 pointLighting = CalculateRawLighting(a_lightSource.base, dirToLight, a_normal, a_viewerDir, a_diffuseSample, a_specularSample);
	
	// Use modified light attenuation equation to get illumination scale for fragment, sourced from: https://imdoingitwrong.wordpress.com/2011/01/31/light-attenuation/
	float illumination	= CalculateIllumination(length(worldFragPos - a_lightSource.position.xyz), a_lightSource.attenuation);
	pointLighting		*= illumination;

	// Calculate and return final fragment color after applying phong point lighting
	return pointLighting;
}

/*
	@brief Calculate and return color of fragment after applying spot lighting.
	@param a_lightSource is the spot light to get lighting information from.
	@param a_normal is the direction in which light gets reflected from.
	@param a_viewerDir is the direction from the fragment to the viewer's position.
	@param a_diffuseSample is the color of the sampled texel of the diffuse map for this fragment.
	@param a_specularSample is the color of the sampled texel of the specular map for this fragment.
	@return vec4 containing color information for a fragment lit by a spot light.
*/
vec4 CalculateSpotLighting(GPU_Spot_Light a_lightSource, vec3 a_normal, vec3 a_viewerDir, vec4 a_diffuseSample, vec4 a_specularSample) {
	vec3 dirToLight = normalize(a_lightSource.position.xyz - worldFragPos); 
	
	vec4 spotLighting = CalculateRawLighting(a_lightSource.base, dirToLight, a_normal, a_viewerDir, a_diffuseSample, a_specularSample);

	// Apply illumination based off fragment's proximity to the spotlight cone
	float fragmentAngle = dot(dirToLight, normalize(-a_lightSource.spotDir));							// Get angle of fragment to light in relation to the spot direction. NOTE: Spot direction is negated so fragment is now pointing at light source for the dot product 
	float cutOff		= a_lightSource.spotInnerCosine - a_lightSource.spotOuterCosine;
	float illumination	= clamp((fragmentAngle - a_lightSource.spotOuterCosine) / cutOff, 0.0, 1.0);	// Negative if outside spotlight, higher than 1 if in inner cone and 0-1 around the cone edges

	spotLighting *= illumination;

	return spotLighting;
}

/*
	@brief Extrapolate normal from normal map and transform it into world space with a TBN matrix.
	@return Sampled normal in correct range and in world space.
*/
vec3 CalculateNormal() {
	vec3 N = normalize(worldNormal);
	vec3 T = normalize(worldTangent);

	// Ensure tangent is orthogonal to the normal (90 degrees) by re-orthogonalizing
	T = normalize(T - dot(T, N) * N);

	vec3 B = cross(N, T) * bitangentHandedness;		// Get unknown up axis by getting the cross between the right (tangent)
													// NOTE: Timesed by the handedness to ensure it always forms a right-handed system with the other axis	
	// Sample normal map
	vec3 bumpMapN = texture(material.normalMap, vertTexCoord).rgb;

	bumpMapN = normalize(2.0 * bumpMapN - 1.f);		// Convert sampled normal from color range (0-1) to normal range (-1-1)
	
	// Convert sampled normal to world space
	vec3 finalN;
	mat3 TBN = mat3(T, B, N);
	
	finalN = TBN * bumpMapN;
	finalN = normalize(finalN);

	return finalN;
}

/*
	@brief Set global parameters for lighting including direction to viewer and texel samples.
	@param a_diffuseSample is the diffuse sample to set.
	@param a_specularSample is the specular sample to set.
	@param a_normalSample is the normal sample to set.
	@param a_dirToViewer is the direction from the fragment pos to the viewer to set.
	@return void.
*/
void SetLightingParameters(inout vec4 a_diffuseSample, inout vec4 a_specularSample, inout vec3 a_normalSample, inout vec3 a_dirToViewer) {
	// Calculate lighting parameters
	a_diffuseSample = vec4(0.6f, 0.2f, 0.7f, 1.f);	// Set to default 'texture not found' color
	if (material.useDiffuseMap == true) { a_diffuseSample = texture(material.diffuseMap, vertTexCoord); }

	a_specularSample = vec4(1);						// Set to white color so same specular applies to all fragments
	if (material.useSpecularMap == true) { a_specularSample = texture(material.specularMap, vertTexCoord); }

	//// Normal map
	a_normalSample = worldNormal;					// Set to default interpolated normal in world space
	if (material.useNormalMap == true) { a_normalSample = CalculateNormal(); }

	a_dirToViewer = normalize(worldViewerPos - worldFragPos);		// Fragment pos -> viewer
}

