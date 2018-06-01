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

uniform sampler2D textureSample0;

struct GPU_Light {
	vec4 position;

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

void main() {
	// Calculate color from sampled texel
	vec4 texelColor = texture(textureSample0, vertTexCoord);	//vec4(1.0f, 0.5f, 0.31f, 1.f);
 
 #if 0	/// Non-struct method 
	// Calculate ambient
	vec4 ambient = ambientCoefficient * lightColor;

	// Calculate diffuse
	vec4 lightDir				= normalize(lightPos - fragPos);
	float diffuseCoefficient	= max(dot(vertNormal, lightDir), 0.0f);

	vec4 diffuse = diffuseCoefficient * lightColor;

	// Calculate specular
	vec4	viewDir		= normalize(viewerPos - fragPos);		// Frag position -> viewer
	vec4	reflectDir	= reflect(-lightDir, vertNormal);		// Negate light dir because reflect expects light source -> frag position
	float	highlight	= pow(max(dot(viewDir, reflectDir), 0.0), shininessCoefficient);
	
	vec4 specular = specularCoefficient * highlight * lightColor;

	// Calculate final colorw ith ambient, diffuse, and specular lighting
	fragColor = (ambient + diffuse + specular) * texelColor;
#else	/// Struct method
	// Calculate ambient
	vec4 finalAmbient = light.ambient * material.ambientColor;

	// Calculate diffuse
	vec4 lightDir				= normalize(light.position - fragPos);
	float diffuseCoefficient	= max(dot(vertNormal, lightDir), 0.0f);		// How much surface is impacted by direction of light

	vec4 finalDiffuse = light.diffuse * (material.diffuseColor * diffuseCoefficient);

	// Calculate specular
	vec4 viewDir		= normalize(viewerPos - fragPos);
	vec4 reflectDir		= reflect(-lightDir, vertNormal);
	float highlight		= pow(max(dot(viewDir, reflectDir), 0.0), material.shininessCoefficient);

	vec4 finalSpecular = light.specular * (material.specular * highlight);

	// Calculate final color with phong lighting
	fragColor = (finalAmbient + finalDiffuse + finalSpecular) * texelColor;
#endif
}