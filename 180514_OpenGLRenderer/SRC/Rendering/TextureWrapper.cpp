#include "TextureWrapper.h"

#define STB_IMAGE_IMPLEMENTATION	// Modify image loading header file to include relevant source code, like including a .cpp
#include <stb/stb_image.h>

#include <iostream>
#include <gl_core_4_4.h>

/**
*	@brief Load a texture file and create a texture on the GPU from the data.
*	@param a_filePath is the path to the texture file, including its extension.
*	@param a_type is the type of texture that is being loaded in (e.g. TEXTURE_DIFFUSE).
*/
TextureWrapper::TextureWrapper(const char * a_filePath, const std::string& a_type)
{
	static char currTexUnit = 0;

	m_type = a_type;

	std::string pathStr = a_filePath;
	m_fileName = pathStr.substr(pathStr.find_last_of('/') + 1, pathStr.size());		// Get file name by getting sub string from last backslash (not inclusive) to end

	// Attempt to load texture data
	stbi_set_flip_vertically_on_load(true);		// Images usually expect 0.0 to be the top of the y axis which is the opposite of OpenGL

	m_texData = stbi_load(a_filePath, &m_texWidth, &m_texHeight, &m_channelNum, 0);

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

	// Check if texture unit is in a valid range
	GLenum texUnitEnum = GL_TEXTURE0 + currTexUnit;

	try {
		if (!(texUnitEnum >= GL_TEXTURE0 && texUnitEnum <= GL_MAX_TEXTURE_IMAGE_UNITS)) {	// Outside of range [0-max texture units on frag shader] inc.
			char errorMsg[256];
			sprintf_s(errorMsg, "ERROR::RENDER_TEXTURE::INVALID_TEXTURE_UNIT");

			// Free texture data
			stbi_image_free(m_texData);

			throw std::runtime_error(errorMsg);
		}
	}
	catch (std::exception const& e) { std::cout << "Exception: " << e.what() << std::endl; }

	// Hold onto assigned texture unit
	m_textureUnit = currTexUnit;
	currTexUnit++;		// Increment to next texture unit for the next texture wrapper object

	// Activate corresponding texture unit so that binding the texture sets it to that texture unit address
	glActiveTexture(GetTexUnitEnum());
	glBindTexture(GL_TEXTURE_2D, *this);

	// Set texture data
	GLenum format;			// Determine format based off the number of channels in the image (e.g. 3 channels = RGB for stuff like .jpgs)

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
}

TextureWrapper::~TextureWrapper()
{
	// Clean up texture data
	stbi_image_free(m_texData);
}

/**
*	@brief Activate bilinear filtering techniques in upscaling and downscaling textures.
*	@return void.
*/
void TextureWrapper::EnableFiltering()
{
	// Bind texture to modify its parameters
	glBindTexture(GL_TEXTURE_2D, *this);

	// (GL_NEAREST = take color of pixel whose center is closest to texture coord, GL_LINEAR = get average color from neighboring pixels to texture coord)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);			// Filtering mode for scaling down textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);			// Filtering mode for scaling up textures (Bilinear)
}

/**
*	@brief Generate mip maps and enable downscaling mipmap linear technique on texture.
*	NOTE: If filtering is going to be enabled then this must be called after or else the downscaling filtering will be overridden.
*	@return void.
*/
void TextureWrapper::EnableMipmapping()
{
	glBindTexture(GL_TEXTURE_2D, *this);

	// NOTE: This technique is for downscaling only, setting a mipmap method to the magnification filter will do nothing and will generate an openGL error
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);		// Interpolate between two closest mipmaps needed for scenario, and then get color from image with linear filtering

	// (Create collection of images from one image at varying degrees of resolution to avoid artifacts when viewing high resolution textures from far away)
	glGenerateMipmap(GL_TEXTURE_2D);
}

/**
*	@brief Enable un-normalised texture coordinates to wrap back around, repeating the image.
*	@return void.
*/
void TextureWrapper::EnableWrapping()
{
	glBindTexture(GL_TEXTURE_2D, *this);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);		// 2D Texture wrap mode on x axis
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);		// 2D Texture wrap mode on y axis
}

/**
*	@brief Get texture unit number (0-32)
*	@return unsigned char representing texture unit.
*/
unsigned char TextureWrapper::GetTexUnit()
{
	return m_textureUnit;
}

/**
*	@brief Get corresponding enum for texture unit (GL_TEXTURE0-GL_TEXTURE31)
*	@return unsigned int representing openGL texture unit enum.
*/
unsigned int TextureWrapper::GetTexUnitEnum()
{
	return GL_TEXTURE0 + m_textureUnit;
}

std::string TextureWrapper::GetType()
{
	return m_type;
}

std::string TextureWrapper::GetFileName()
{
	return m_fileName;
}
