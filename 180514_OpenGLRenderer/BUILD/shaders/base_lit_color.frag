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

void main() {
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

	// Calculate final color with ambient, diffuse, and specular lighting
	fragColor = (ambient + diffuse + specular) * vertColor;
}