#include "ShaderWrapper.h"
#include "Renderer_Utility_Literals.h"
#include "Renderer_Utility_Funcs.h"
#include "Texture\Texture.h"
#include "Light\PhongLight_Dir.h"
#include "Light\PhongLight_Spot.h"
#include "Light\PhongLight_Point.h"
#include "Mesh.h"

#include <gl_core_4_4.h>
#include <glm/ext.hpp>
#include <string>

namespace SPRON {

	ShaderWrapper::ShaderWrapper()
	{
		// Create shader program on GPU and get unique id from openGL
		m_ID = glCreateProgram();
	}

	ShaderWrapper::~ShaderWrapper()
	{
		glDeleteProgram(*this);
	}

	/**
	*	@brief Create shader from text file and attach to shader program.
	*	@param a_filePath is the path to the shader file, including the extension.
	*	@param a_shaderType is the enum specifying what kind of shader it is.
	*	@param a_headerStr is an optional string that is attached to the front of the shader source so the shader can read global functions and variables.
	*	@return void.
	*/
	void ShaderWrapper::LoadShader(const char * a_filePath, unsigned int a_shaderType, const char* a_headerStr)
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
		if (a_headerStr) {		// Header string included, attach to front of shader source
			const char* shaderStrings[2] = { a_headerStr, shaderSource };

			glShaderSource(newShaderID, 2, shaderStrings, NULL);
		}
		else {					// No header string included
			glShaderSource(newShaderID, 1, &shaderSource, NULL);	// Keep length as null to grab the entire string
		}

		glCompileShader(newShaderID);

		// Attach and add created shader ID to list of attached shaders
		glAttachShader(*this, newShaderID);
		m_shaderIDs.push_back(newShaderID);
	}

	/**
	*	@brief Link all attached shaders together to form complete shader program.
	*	@return void.
	*/
	void ShaderWrapper::LinkShaders()
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
	void ShaderWrapper::SetBool(const char * a_name, bool a_val)
	{
		// Bind shader program
		glUseProgram(*this);

		glUniform1i(FindLocation(a_name), int(a_val));
	}

	void ShaderWrapper::SetInt(const char * a_name, int a_val)
	{
		// Bind shader program
		glUseProgram(*this);

		glUniform1i(FindLocation(a_name), a_val);
	}

	void ShaderWrapper::SetFloat(const char * a_name, float a_val)
	{
		// Bind shader program
		glUseProgram(*this);

		glUniform1f(FindLocation(a_name), a_val);
	}

	void ShaderWrapper::SetVec3(const char * a_name, const glm::vec3 & a_val)
	{
		// Bind shader program
		glUseProgram(*this);

		glUniform3f(FindLocation(a_name), a_val.x, a_val.y, a_val.z);
	}

	void ShaderWrapper::SetVec4(const char * a_name, const glm::vec4 & a_val)
	{
		// Bind shader program
		glUseProgram(*this);

		glUniform4f(FindLocation(a_name), a_val.x, a_val.y, a_val.z, a_val.w);
	}

	void ShaderWrapper::SetTexture(const char * a_name, TextureWrapperBase * a_tex)
	{
		if (a_tex == nullptr) { return; }	// Texture is null, do not assign

		glUseProgram(*this);

		// Set specified sampler2D to texture unit
		SetInt(a_name, a_tex->GetTexUnit());
	}

	void ShaderWrapper::SetMat4(const char * a_name, const glm::mat4 & a_val)
	{
		glUseProgram(*this);

		glUniformMatrix4fv(FindLocation(a_name), 1, GL_FALSE, glm::value_ptr(a_val));	// Convert GLM matrix 4 to float pointer to be compatable with openGL
	}

	void ShaderWrapper::SetMaterial(const char * a_name, Material a_mat)
	{
		glUseProgram(*this);

		std::string nameStr = a_name;

		SetVec4((nameStr + ".ambientColor").c_str(), a_mat.ambientColor);
		SetVec4((nameStr + ".diffuseColor").c_str(), a_mat.diffuseColor);
		SetVec4((nameStr + ".specular").c_str(), a_mat.specular);
		SetFloat((nameStr + ".shininessCoefficient").c_str(), a_mat.shininessCoefficient);
		SetTexture((nameStr + ".diffuseMap").c_str(), a_mat.diffuseMap);
		SetTexture((nameStr + ".specularMap").c_str(), a_mat.specularMap);

		// Inform GPU whether diffuse and specular maps are valid
		SetBool((nameStr + ".useDiffuseMap").c_str(), (a_mat.diffuseMap && a_mat.diffuseMap->IsNotNull() ? true : false));
		SetBool((nameStr + ".useSpecularMap").c_str(), (a_mat.specularMap && a_mat.specularMap->IsNotNull() ? true : false));

#if DISABLE_SPECULAR_MAPS
		SetBool((nameStr + ".useSpecularMap").c_str(), false);
#endif
	}

	void ShaderWrapper::SetBaseLight(const char * a_name, PhongLight * a_light)
	{
		glUseProgram(*this);

		std::string nameStr = a_name;		// Convert to string in order to easily concatenate with variable nmaes

		SetVec4((nameStr + ".base.ambient").c_str(), a_light->GetAmbient());
		SetVec4((nameStr + ".base.diffuse").c_str(), a_light->GetDiffuse());
		SetVec4((nameStr + ".base.specular").c_str(), a_light->GetSpecular());
	}

	void ShaderWrapper::SetDirectionalLight(const char * a_name, PhongLight_Dir * a_light)
	{
		glUseProgram(*this);

		std::string nameStr = a_name;		// Convert to string in order to easily concatenate with variable nmaes

		SetBaseLight(a_name, a_light);
		SetVec4((nameStr + ".castDir").c_str(), a_light->GetCastDir());
	}

	void ShaderWrapper::SetSpotLight(const char * a_name, PhongLight_Spot * a_light)
	{
		glUseProgram(*this);

		std::string nameStr = a_name;		// Convert to string in order to easily concatenate with variable nmaes

		SetBaseLight(a_name, a_light);
		SetVec4((nameStr + ".position").c_str(), a_light->GetPos());
		SetVec4((nameStr + ".spotDir").c_str(), a_light->GetSpotDir());
		SetFloat((nameStr + ".spotInnerCosine").c_str(), a_light->GetSpotInnerCosine());
		SetFloat((nameStr + ".spotOuterCosine").c_str(), a_light->GetSpotOuterCosine());
	}

	void ShaderWrapper::SetPointLight(const char * a_name, PhongLight_Point * a_light)
	{
		glUseProgram(*this);

		std::string nameStr = a_name;		// Convert to string in order to easily concatenate with variable nmaes

		SetBaseLight(a_name, a_light);
		SetVec4((nameStr + ".position").c_str(), a_light->GetPos());
		SetFloat((nameStr + ".attenuation.illuminationRadius").c_str(), a_light->GetIlluminationRadius());
		SetFloat((nameStr + ".attenuation.minIllumination").c_str(), a_light->GetMinIllumination());
	}

	/**
	*	@brief Attempt to find corresponding uniform variable in shader program.
	*	@param a_name is the name of the uniform variable to find.
	*	@return integer ID of the uniform variable location, or -1 if unable to find it.
	*/
	int ShaderWrapper::FindLocation(const char * a_name)
	{
		// Bind as current shader program in order to search for the kernel
		glUseProgram(*this);

		int foundKernel = glGetUniformLocation(*this, a_name);

		// Error handling
#if ERROR_CHECK_UNIFORM_FIND
		try {
			if (foundKernel == -1) {		// Unable to find kernel
				int boundProgramID; glGetIntegerv(GL_CURRENT_PROGRAM, &boundProgramID);

				char errorMsg[256];
				sprintf_s(errorMsg, "ERROR::MATERIAL::FAILED_TO_FIND: %s in %i with %i bound \nDescription: Uniform name not found in bound material, it was either misspelled or optimized away for not being used in the shader.",
					a_name, m_ID, boundProgramID);

				throw std::runtime_error(errorMsg);
			}
		}
		catch (std::exception const& e) { 
			std::cout << "Exception: " << e.what() << std::endl; 
			__debugbreak();
		}
#endif 

		return foundKernel;
	}
}

