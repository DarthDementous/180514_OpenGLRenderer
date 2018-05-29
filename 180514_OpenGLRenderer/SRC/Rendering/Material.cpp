#include "Material.h"
#include "Renderer_Utility_Literals.h"
#include "Renderer_Utility_Funcs.h"
#include "TextureWrapper.h"

#include <gl_core_4_4.h>

Material::Material()
{
	// Create shader program on GPU and get unique id from openGL
	m_ID = glCreateProgram();
}

Material::~Material()
{
	glDeleteProgram(*this);
}

/**
*	@brief Create shader from text file and attach to shader program.
*	@param a_filePath is the path to the shader file, including the extension.
*	@param a_shaderType is the enum specifying what kind of shader it is.
*	@return void.
*/
void Material::LoadShader(const char * a_filePath, unsigned int a_shaderType)
{
	// Convert from text file into shader source
	std::string shaderString;
	RendererUtility::LoadTextToString(a_filePath, shaderString);

	const char* shaderSource = shaderString.c_str();

	// Create appropriate shader framework
	unsigned int newShaderID;

	switch (a_shaderType) {
		case VERT_SHADER:
			newShaderID = glCreateShader(GL_VERTEX_SHADER);
			break;
		case FRAG_SHADER:
			newShaderID = glCreateShader(GL_FRAGMENT_SHADER);
			break;
		default:
			assert(false && "ERROR::SHADER_PROGRAM::UNRECOGNISED_SHADER_TYPE");
	}

	// Set shader source and attempt to compile
	glShaderSource(newShaderID, 1, &shaderSource, NULL);
	glCompileShader(newShaderID);

	// Attach and add created shader ID to list of attached shaders
	glAttachShader(*this, newShaderID);
	m_shaderIDs.push_back(newShaderID);
}

/**
*	@brief Link all attached shaders together to form complete shader program.
*	@return void.
*/
void Material::LinkShaders()
{
	glLinkProgram(*this);

	// After linking shaders they are no longer needed, delete them and clear IDs to reflect this
	for (int i = 0; i < m_shaderIDs.size(); ++i) {
		glDeleteShader(m_shaderIDs[i]);
	}

	m_shaderIDs.clear();
}

/**
*	@brief Attempt to find uniform variable linked to shader program and set to given value.
*	@param a_name is the name of the uniform variable.
*	@param a_val is the value to apply to the found uniform variable.
*	@return void.
*/
void Material::SetBool(const char * a_name, bool a_val)
{
	// Bind shader program
	glUseProgram(*this);

	int foundKernel = glGetUniformLocation(*this, a_name);			// Returns int in case it can't find location (-1)
	glUniform1i(foundKernel, int(a_val));							
}

void Material::SetInt(const char * a_name, int a_val)
{
	// Bind shader program
	glUseProgram(*this);

	int foundKernel = glGetUniformLocation(*this, a_name);			
	glUniform1i(foundKernel, a_val);								
}

void Material::SetFloat(const char * a_name, float a_val)
{
	// Bind shader program
	glUseProgram(*this);

	int foundKernel = glGetUniformLocation(*this, a_name);
	glUniform1f(foundKernel, a_val);
}

void Material::SetVec3(const char * a_name, const glm::vec3 & a_val)
{
	// Bind shader program
	glUseProgram(*this);

	int foundKernel = glGetUniformLocation(*this, a_name);
	glUniform3f(foundKernel, a_val.x, a_val.y, a_val.z);
}

void Material::SetVec4(const char * a_name, const glm::vec4 & a_val)
{
	// Bind shader program
	glUseProgram(*this);

	int foundKernel = glGetUniformLocation(*this, a_name);
	glUniform4f(foundKernel, a_val.x, a_val.y, a_val.z, a_val.w);
}

void Material::SetTexture(const char * a_name, TextureWrapper * a_tex)
{
	glUseProgram(*this);

	// Set specified sampler2D to texture unit
	SetInt(a_name, a_tex->GetTexUnit());
}


