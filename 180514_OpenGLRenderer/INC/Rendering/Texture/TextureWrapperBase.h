#pragma once

namespace SPRON {
	/**
	*	@brief Pure virtual class for openGL texture wrappers.
	*/
	class TextureWrapperBase {
	public:
		TextureWrapperBase();
		virtual ~TextureWrapperBase() = 0;

		unsigned char	GetTexUnit();
		unsigned int	GetTexUnitEnum();

		operator unsigned int() { return m_ID; }
	protected:
		unsigned int m_ID;				// Location ID for texture object
		unsigned char m_textureUnit;	// Active texture unit that texture object has been bound to
	private:
	};
}