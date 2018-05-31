#version 440 core
out vec4 fragColor;

in vec4 vertColor;					// Setting input with same name as output from another shader allows data to be shared
in vec4 vertPos;
in vec4 vertNormal;
in vec2 vertTexCoord;

in vec4 fragPos;

uniform float	ambientCoefficient;	// How strong the ambient light is
uniform vec4	lightColor;
uniform vec4	lightPos;

uniform float	specularCoefficient;	// How strong the specular highlights are
uniform float	shininessCoefficient;	// How sharp the curve of the highlight is [lower = highlight is larger and more spread out, higher = highlight is small and narrow]
uniform vec4	viewPos;				// Viewer position in global space, usually camera position

uniform sampler2D textureSample0;	// If not manually set, corresponds to texture unit '0' which is activated by default

void main() {
	// Calculate color from sampled texel
	vec4 texelColor = texture(textureSample0, vertTexCoord);

	// Calculate ambient
	vec4 ambient = ambientCoefficient * lightColor;

	// Calculate diffuse
	vec4	lightDir = normalize(lightPos - fragPos);
	float	diffuseCoefficient = max(dot(vertNormal, lightDir), 0.0f);	// Calculate scale of diffuse light, zeroing out if light dir is behind frag pos
	
	vec4	diffuse = diffuseCoefficient * lightColor;

	// Calculate specular
	vec4	viewDir		= normalize(viewPos - fragPos);		// Viewer -> frag position
	vec4	reflectDir	= reflect(-lightDir, vertNormal);	// Negate light direction because reflect expects light source -> frag position
	float	highlight	= pow(max(dot(viewDir, reflectDir), 0.0), shininessCoefficient);
	
	vec4	specular = specularCoefficient * shininessCoefficient * lightColor;

	// Calculate final color with ambient, diffuse, and specular lighting
	fragColor = (ambient + diffuse + specular) * texelColor;
}