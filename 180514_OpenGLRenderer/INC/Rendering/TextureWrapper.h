#pragma once

class TextureWrapper {
public:
	TextureWrapper(const char* a_filePath, unsigned char a_textureUnit, bool a_hasAlpha = false);
	~TextureWrapper();

	void EnableFiltering();
	void EnableMipmapping();
	void EnableWrapping();

	unsigned char	GetTexUnit();
	unsigned int	GetTexUnitEnum();

	operator unsigned int() { return m_ID; }
protected:
private:
	unsigned int m_ID;

	// Texture info
	int m_texWidth;
	int m_texHeight;
	int m_channelNum;

	unsigned char*	m_texData;
	unsigned char	m_textureUnit;		// OpenGL supports maximum of 32 texture samplers in a shader, therefore range has to be 0-31
};