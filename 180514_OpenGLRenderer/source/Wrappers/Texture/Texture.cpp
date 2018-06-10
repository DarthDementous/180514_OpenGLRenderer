#include "Texture/Texture.h"

#define STB_IMAGE_IMPLEMENTATION	// Modify image loading header file to include relevant source code, like including a .cpp
#include <stb/stb_image.h>

#include <iostream>
#include <gl_core_4_4.h>

namespace SPRON {

	/**
	*	@brief Load a texture file and create a texture on the GPU from the data.
	*	@param a_filePath is the path to the texture file, including its extension.
	*	@param a_type is the type of texture that is being loaded in (e.g. TEXTURE_DIFFUSE).
	*/
	Texture::Texture(const char * a_filePath, const std::string& a_type, eFilteringOption a_filterOption) : TextureWrapperBase() // Assign valid texture unit
	{
		m_type = a_type;

		std::string pathStr = a_filePath;
		m_fileName = pathStr.substr(pathStr.find_last_of('/') + 1, pathStr.size());		// Get file name by getting sub string from last backslash (not inclusive) to end

		// Attempt to load texture data
		stbi_set_flip_vertically_on_load(true);		// Images usually expect 0.0 to be the top of the y axis which is the opposite of OpenGL

		m_texData = stbi_load(a_filePath, &m_texWidth, &m_texHeight, &m_channelNum, 0);

		// Error handling
		try {
			if (!m_texData) {
				char errorMsg[256];
				sprintf_s(errorMsg, "ERROR::RENDER_TEXTURE::FAILED_TO_LOAD: %s", a_filePath);

				throw std::runtime_error(errorMsg);
			}
		}
		catch (std::exception const& e) { std::cout << "Exception: " << e.what() << std::endl; }

		// Create texture on GPU
		glGenTextures(1, &m_ID);

		// Activate corresponding texture unit so that binding the texture sets it to that texture unit address
		glActiveTexture(GetTexUnitEnum());
		glBindTexture(GL_TEXTURE_2D, *this);

		// Set texture data
		GLenum format = GL_RGB;			// Determine format based off the number of channels in the image (e.g. 3 channels = RGB for stuff like .jpgs)

		if (m_channelNum == 1) { format = GL_RED; }
		if (m_channelNum == 3) { format = GL_RGB; }
		if (m_channelNum == 4) { format = GL_RGBA; }

		glTexImage2D(	// NOTE: Applies to currently bound texture
			GL_TEXTURE_2D,		// Enum for texture dimension
			0,					// Mipmap level (0 by default)
			format,				// Format to STORE texture in
			m_texWidth,			// Width of texture (pixels)
			m_texHeight,		// Height of texture (pixels)
			0,					// Legacy parameter, must be 0
			format,				// Format of SOURCE texture
			GL_UNSIGNED_BYTE,	// Type of data in source texture
			m_texData);			// Memory location of texture data

		// Account for one channel specular images
		if (m_type == "texture_specular") {

			GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ALPHA };		// Map red channel across color channels so shaders interpret grayscale
			glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
		}

		// Set texture attributes
		switch (a_filterOption) {
			case FILTERING_LINEAR:
				EnableFiltering();
				break;
			case FILTERING_MIPMAP:
				EnableMipmapping();
				break;
		}

		EnableWrapping();

	}

	Texture::~Texture()
	{
		// Clean up texture data
		stbi_image_free(m_texData);

		// Clean up openGL texture object
		glDeleteTextures(1, &m_ID);
	}

	/**
	*	@brief Activate bilinear filtering techniques in upscaling and downscaling textures.
	*	@return void.
	*/
	void Texture::EnableFiltering()
	{
		// Bind texture to modify its parameters
		glBindTexture(GL_TEXTURE_2D, *this);

		// (GL_NEAREST = take color of pixel whose center is closest to texture coord, GL_LINEAR = get average color from neighboring pixels to texture coord)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);			// Filtering mode for scaling down textures
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);			// Filtering mode for scaling up textures (Bilinear)
	}

	/**
	*	@brief Generate mip maps and enable downscaling mipmap linear technique and upscale linear filtering on texture.
	*	NOTE: This is enabled as a more intensive but nicer looking option to enabling just linear filtering.
	*	@return void.
	*/
	void Texture::EnableMipmapping()
	{
		glBindTexture(GL_TEXTURE_2D, *this);

		// NOTE: This technique is for downscaling only, setting a mipmap method to the magnification filter will do nothing and will generate an openGL error
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);		// Interpolate between two closest mipmaps needed for scenario, and then get color from image with linear filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// (Create collection of images from one image at varying degrees of resolution to avoid artifacts when viewing high resolution textures from far away)
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	/**
	*	@brief Enable un-normalised texture coordinates to wrap back around, repeating the image.
	*	@return void.
	*/
	void Texture::EnableWrapping()
	{
		glBindTexture(GL_TEXTURE_2D, *this);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);		// 2D Texture wrap mode on x axis
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);		// 2D Texture wrap mode on y axis
	}

	std::string Texture::GetType()
	{
		return m_type;
	}

	std::string Texture::GetFileName()
	{
		return m_fileName;
	}

	bool Texture::IsNotNull()
	{
		return m_texData;		// Will return true if texture data isn't nullptr
	}
}