#pragma once

#include <string>

enum eFilteringOption {
	FILTERING_MIPMAP,
	FILTERING_LINEAR
};

class TextureWrapper {
public:
	TextureWrapper(const char* a_filePath, const std::string& a_type, eFilteringOption a_filterOption);
	~TextureWrapper();

	void EnableFiltering();
	void EnableMipmapping();
	void EnableWrapping();

	unsigned char	GetTexUnit();
	unsigned int	GetTexUnitEnum();
	std::string		GetType();
	std::string		GetFileName();

	operator unsigned int() { return m_ID; }
protected:
private:
	unsigned int m_ID;

	// Texture info
	int m_texWidth;
	int m_texHeight;
	int m_channelNum;
	std::string m_type;					// Type name of the texture stored as a string for easy concatenation e.g. "texture_diffuse"
	std::string m_fileName;				// Hold onto file name to compare against other textures

	unsigned char*	m_texData;
	unsigned char	m_textureUnit;		// OpenGL supports maximum of 32 texture samplers in a shader, therefore range has to be 0-31
};