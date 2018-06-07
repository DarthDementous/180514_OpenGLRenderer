#pragma once

#include <string>
#include "Texture/TextureWrapperBase.h"

namespace SPRON {
	enum eFilteringOption {
		FILTERING_MIPMAP,
		FILTERING_LINEAR
	};

	class Texture : public TextureWrapperBase {
	public:
		Texture(const char* a_filePath, const std::string& a_type, eFilteringOption a_filterOption);
		virtual ~Texture();

		void EnableFiltering();
		void EnableMipmapping();
		void EnableWrapping();

		std::string		GetType();
		std::string		GetFileName();

		bool IsNotNull();
	protected:
	private:
		// Texture info
		int m_texWidth;
		int m_texHeight;
		int m_channelNum;
		std::string m_type;					// Type name of the texture stored as a string for easy concatenation e.g. "texture_diffuse"
		std::string m_fileName;				// Hold onto file name to compare against other textures

		unsigned char*	m_texData;
	};
}
