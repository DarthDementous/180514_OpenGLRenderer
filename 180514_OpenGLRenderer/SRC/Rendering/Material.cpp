#include "Material.h"
#include "Renderer_Utility_Literals.h"
#include "Renderer_Utility_Funcs.h"

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
