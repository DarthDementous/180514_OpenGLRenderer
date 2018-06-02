#version 440 core
out vec4 fragColor;

in vec4 vertColor;		
in vec4 vertPos;
in vec4 vertNormal;
in vec2 vertTexCoord;

in vec4 fragPos;

uniform float ambientCoefficient;	// How strong the ambient light is
uniform vec4 lightColor;
uniform vec4 lightPos;

uniform float specularCoefficient;	// How strong the specular highlights are
uniform float shininessCoefficient;	// How sharp the curve of the specular highlights are [lower = softer and more spread out, higher = sharper and narrower]
uniform vec4 viewerPos;

uniform sampler2D diffuseMap;		// Texture artistically created with baked in lighting
uniform sampler2D specularMap;		// Texture in a one color spectrum that defines the highlight points for specular

struct GPU_Dir_Light {
	vec4 castDir;			// Must have a w component of 0, indicates direction of light ray to objects

	// NOTE: Controls color and intensity 
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};
uniform GPU_Dir_Light dirLight;

struct GPU_Pt_Light {
	vec4 position;		// Must have a w component of 1

	float illuminationRadius;	// Coverage distance of light
	float minIllumination;		// The minimum illumination of the light before it is cut-off, applied across the illumination radius (like an epsilon)

	// NOTE: Controls color and intensity 
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};

#define NUM_POINT_LIGHT 2
uniform GPU_Pt_Light pointLights[NUM_POINT_LIGHT];

struct GPU_Spot_Light {
	vec4	position;
	vec4	spotDir;						// Direction spotlight is aiming in
	float	spotInnerCosine;				// The cosine of the angle that specifies the radius of the cone in which objects are lit
	float	spotOuterCosine;

	// NOTE: Controls color and intensity 
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};
uniform GPU_Spot_Light spotLight;

struct GPU_Light {
	//vec4 position;
	vec4 vector;					// Point-based or directional depending on the w component

	// [SPOT LIGHT]
	float illuminationRadius;		// Coverage distance of light
	float minIllumination;			// The minimum illumination of the light before it is cut-off, applied across the illumination radius (like an epsilon)

	// NOTE: Controls color and intensity 
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};

uniform GPU_Light light;

struct GPU_Material {		// Serves as a container of uniform variables, accessed via the instance name and then the variable e.g. "material.ambient"
	vec4	ambientColor;			// Color of object in response to ambient lighting 
	vec4	diffuseColor;			// Color of object in response to diffuse lighting	(usually same as ambient)
	vec4	specular;				// Color and intensity of object in response to specular lighting (R, G and B values are usually the same)
	float	shininessCoefficient;	// How sharp the curve of the specular highlights are [lower = softer and more spread out, higher = sharper and narrower]
};

uniform GPU_Material material;

/*
	@brief Calculate and return color of fragment after applying directional lighting.
	@param a_lightSource is the directional light to get lighting information from.
	@param a_normal is the direction in which light gets reflected from.
	@param a_viewerDir is the direction from the fragment to the viewer's position.
	@param a_diffuseSample is the color of the sampled texel of the diffuse map for this fragment.
	@param a_specularSample is the color of the sampled texel of the specular map for this fragment.
	@return vec4 containing color information for a fragment lit by a directional light.
*/
vec4 CalculateDirectionalLighting(GPU_Dir_Light a_lightSource, vec4 a_normal, vec4 a_viewerDir, vec4 a_diffuseSample, vec4 a_specularSample) {
	// Calculate ambient
	vec4 finalAmbient = a_lightSource.ambient * material.ambientColor * a_diffuseSample;

	// Calculate diffuse
	float	diffuseScale	= max(dot(a_normal, -a_lightSource.castDir), 0.0f);	// How much fragment is impacted by the light based off its direction (fragment -> light source)
	vec4	finalDiffuse	= a_lightSource.diffuse * diffuseScale * material.diffuseColor * a_diffuseSample;

	// Calculate specular
	vec4	reflectDir		= reflect(a_lightSource.castDir, a_normal);
	float	specularScale	= pow(max(dot(a_viewerDir, reflectDir), 0.0), material.shininessCoefficient);	// Like with diffuse, if dot product is negative then viewer cannot see specular
	vec4	finalSpecular	= a_lightSource.specular * specularScale * material.specular * a_specularSample;

	// Calculate and return final fragment color after applying directional phong lighting
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
vec4 CalculatePointLighting(GPU_Pt_Light a_lightSource, vec4 a_normal, vec4 a_viewerDir, vec4 a_diffuseSample, vec4 a_specularSample) {
	vec4 dirToLight = normalize(a_lightSource.position - fragPos);

	// Calculate ambient
	vec4 finalAmbient = a_lightSource.ambient * material.ambientColor * a_diffuseSample;

	// Calculate diffuse
	float	diffuseScale	= max(dot(a_normal, dirToLight), 0.0f);	// How much fragment is impacted by the light based off its direction (fragment -> light source)
	vec4	finalDiffuse	= a_lightSource.diffuse * diffuseScale * material.diffuseColor * a_diffuseSample;

	// Calculate specular
	vec4	 reflectDir		= reflect(-dirToLight, a_normal);		// Reflect expects light source -> fragment therefore it must be negated
	float	specularScale	= pow(max(dot(a_viewerDir, reflectDir), 0.0), material.shininessCoefficient);	// Like with diffuse, if dot product is negative then viewer cannot see specular
	vec4	finalSpecular	= a_lightSource.specular * specularScale * material.specular * a_specularSample;

	// Use modified light attenuation equation to get illumination scale for fragment, sourced from: https://imdoingitwrong.wordpress.com/2011/01/31/light-attenuation/
	float lightDist			= length(a_lightSource.position - fragPos);
	float denominator		= lightDist / a_lightSource.illuminationRadius + 1;
	float illumination		= 1 / (denominator * denominator);

	// Apply attenuation rules to illumination
	illumination = (illumination - a_lightSource.minIllumination) / (1 - a_lightSource.minIllumination);	// 1. Beyond illumination radius, illumination becomes 0
	illumination = max(illumination, 0);																	// 2. Illumination is at full intensity when distance from light is 0

	finalAmbient	*= illumination;
	finalDiffuse	*= illumination;
	finalSpecular	*= illumination;

	// Calculate and return final fragment color after applying phong point lighting
	return (finalAmbient + finalDiffuse + finalSpecular);
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
vec4 CalculateSpotLighting(GPU_Spot_Light a_lightSource, vec4 a_normal, vec4 a_viewerDir, vec4 a_diffuseSample, vec4 a_specularSample) {
	vec4 dirToLight = normalize(a_lightSource.position - fragPos);

	// Calculate ambient
	vec4 finalAmbient = a_lightSource.ambient * material.ambientColor * a_diffuseSample;

	// Calculate diffuse
	float	diffuseScale	= max(dot(a_normal, dirToLight), 0.0f);	// How much fragment is impacted by the light based off its direction (fragment -> light source)
	vec4	finalDiffuse	= a_lightSource.diffuse * diffuseScale * material.diffuseColor * a_diffuseSample;

	// Calculate specular
	vec4	reflectDir		= reflect(-dirToLight, a_normal);		// Reflect expects light source -> fragment therefore it must be negated
	float	specularScale	= pow(max(dot(a_viewerDir, reflectDir), 0.0), material.shininessCoefficient);	// Like with diffuse, if dot product is negative then viewer cannot see specular
	vec4	finalSpecular	= a_lightSource.specular * specularScale * material.specular * a_specularSample;

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
	//// DEBUGGING: Make sure all relevant uniform variables are in use or else they'll be optimized away

	fragColor = vec4(0);

	// 1. Calculate lighting parameters
	vec4 diffuseSample	= texture(diffuseMap, vertTexCoord);
	vec4 specularSample = texture(specularMap, vertTexCoord);

	vec4 dirToViewer = normalize(viewerPos - fragPos);	// Fragment pos -> viewer

	// 2. Apply directional lighting
	fragColor += CalculateDirectionalLighting( dirLight, vertNormal, dirToViewer, diffuseSample, specularSample);

	// 3. Apply point lighting
	for (int i = 0; i < NUM_POINT_LIGHT; ++i) {
		fragColor += CalculatePointLighting(pointLights[i], vertNormal, dirToViewer, diffuseSample, specularSample);
	}

	// 4. Apply spot lighting
	fragColor += CalculateSpotLighting(spotLight, vertNormal, dirToViewer, diffuseSample, specularSample);
}