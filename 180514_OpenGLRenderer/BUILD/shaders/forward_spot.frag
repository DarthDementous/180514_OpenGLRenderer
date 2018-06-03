#version 440 core

varying vec2 vertTexCoord;			
varying vec4 vertNormal;
varying vec4 fragPos;				// World position of fragment

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

struct GPU_Spot_Light {
	vec4	position;
	vec4	spotDir;						// Direction spotlight is aiming in
	float	spotInnerCosine;				// The cosine of the angle that specifies the radius of the cone in which objects are lit
	float	spotOuterCosine;

	//GPU_Light_Attenuation attenuation;
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
};

uniform GPU_Material material;

uniform GPU_Spot_Light spotLight;

uniform vec4 viewerPos;

/*
	@brief Calculate and return color of fragment after applying spot lighting.
	@param a_lightSource is the spot light to get lighting information from.
	@param a_normal is the direction in which light gets reflected from.
	@param a_viewerDir is the direction from the fragment to the viewer's position.
	@param a_diffuseSample is the color of the sampled texel of the diffuse map for this fragment.
	@param a_specularSample is the color of the sampled texel of the specular map for this fragment.
	@return vec4 containing color information for a fragment lit by a spot light.
*/
vec4 CalculateSpotLighting(GPU_Spot_Light a_lightSource, vec4 a_normal, vec4 a_viewerDir, vec4 a_diffuseSample, vec4 a_specularSample) {
	vec4 dirToLight = normalize(a_lightSource.position - fragPos);

	// Calculate ambient
	vec4 finalAmbient = a_lightSource.base.ambient * material.ambientColor * a_diffuseSample;

	// Calculate diffuse
	float	diffuseScale	= max(dot(a_normal, dirToLight), 0.0f);	// How much fragment is impacted by the light based off its direction (fragment -> light source)
	vec4	finalDiffuse	= a_lightSource.base.diffuse * diffuseScale * material.diffuseColor * a_diffuseSample;

	// Calculate specular
	vec4	reflectDir		= reflect(-dirToLight, a_normal);		// Reflect expects light source -> fragment therefore it must be negated
	float	specularScale	= pow(max(dot(a_viewerDir, reflectDir), 0.0), material.shininessCoefficient);	// Like with diffuse, if dot product is negative then viewer cannot see specular
	vec4	finalSpecular	= a_lightSource.base.specular * specularScale * material.specular * a_specularSample;

	// Apply illumination based off fragment's proximity to the spotlight cone
	float fragmentAngle = dot(dirToLight, normalize(-a_lightSource.spotDir));							// Get angle of fragment to light in relation to the spot direction. NOTE: Spot direction is negated so fragment is now pointing at light source for the dot product 
	float cutOff		= a_lightSource.spotInnerCosine - a_lightSource.spotOuterCosine;
	float illumination	= clamp((fragmentAngle - a_lightSource.spotOuterCosine) / cutOff, 0.0, 1.0);	// Negative if outside spotlight, higher than 1 if in inner cone and 0-1 around the cone edges

	// NOTE: Ambient isn't scaled to ensure that objects outside of cone are still somewhat lit
	finalDiffuse	*= illumination;
	finalSpecular	*= illumination;

	return (finalAmbient + finalDiffuse + finalSpecular);
}

void main() {
	// Calculate lighting parameters
	vec4 diffuseSample = vec4(0.6f, 0.2f, 0.7f, 1.f);	// Set to default 'texture not found' color
	if (material.useDiffuseMap == true) { diffuseSample = texture(material.diffuseMap, vertTexCoord); }

	vec4 specularSample = vec4(1);						// Set to white color so same specular applies to all fragments
	if (material.useSpecularMap == true) { specularSample = texture(material.specularMap, vertTexCoord); }

	vec4 dirToViewer = normalize(viewerPos - fragPos);	// Fragment pos -> viewer

	// Apply point lighting pass
	gl_FragColor = CalculateSpotLighting( spotLight, vertNormal, dirToViewer, diffuseSample, specularSample);
}