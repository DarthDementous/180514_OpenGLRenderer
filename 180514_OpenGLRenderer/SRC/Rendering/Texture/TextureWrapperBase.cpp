#include "Texture\TextureWrapperBase.h"

#include <gl_core_4_4.h>
#include <iostream>

namespace SPRON {
	TextureWrapperBase::TextureWrapperBase()
	{
		static char currTexUnit = 0;

		// Check if texture unit is in a valid range
		GLenum texUnitEnum = GL_TEXTURE0 + currTexUnit;

		try {
			if (!(texUnitEnum >= GL_TEXTURE0 && texUnitEnum <= GL_MAX_TEXTURE_IMAGE_UNITS)) {	// Outside of range [0-max texture units on frag shader] inc.
				char errorMsg[256];
				sprintf_s(errorMsg, "ERROR::RENDER_TEXTURE::INVALID_TEXTURE_UNIT");

				throw std::runtime_error(errorMsg);
			}
		}
		catch (std::exception const& e) { std::cout << "Exception: " << e.what() << std::endl; }

		// Hold onto assigned texture unit
		m_textureUnit = currTexUnit;
		currTexUnit++;			// Increment to next texture unit for the next texture wrapper object
	}

	TextureWrapperBase::~TextureWrapperBase()
	{
	}

	/**
	*	@brief Get texture unit number (0-32)
	*	@return unsigned char representing texture unit.
	*/
	unsigned char TextureWrapperBase::GetTexUnit()
	{
		return m_textureUnit;
	}

	/**
	*	@brief Get corresponding enum for texture unit (GL_TEXTURE0-GL_TEXTURE31)
	*	@return unsigned int representing openGL texture unit enum.
	*/
	unsigned int TextureWrapperBase::GetTexUnitEnum()
	{
		return GL_TEXTURE0 + m_textureUnit;
	}
}