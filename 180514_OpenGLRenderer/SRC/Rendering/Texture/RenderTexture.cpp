#include "Texture/RenderTexture.h"

#include <gl_core_4_4.h>

namespace SPRON {
	RenderTexture::RenderTexture(unsigned int a_width, unsigned int a_height) : TextureWrapperBase()		// Assign valid texture unit
	{
		// Create texture on GPU
		glGenTextures(1, &m_ID);

		// Activate corresponding texture unit so that binding the texture sets it to that texture unit address
		glActiveTexture(GetTexUnitEnum());
		glBindTexture(GL_TEXTURE_2D, *this);

		// Set render texture data (usually dimensions of screen, empty data)
		glTexImage2D(
			GL_TEXTURE_2D,
			0,							// No mipmapping for render textures
			GL_RGB,
			a_width,
			a_height,
			0,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			NULL);

		// Enable filtering (linear)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	}

	RenderTexture::~RenderTexture()
	{
		// Clean up openGL texture object
		glDeleteTextures(1, &m_ID);
	}
}