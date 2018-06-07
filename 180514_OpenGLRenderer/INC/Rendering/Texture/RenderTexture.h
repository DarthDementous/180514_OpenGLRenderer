#pragma once

#include "Texture/TextureWrapperBase.h"

namespace SPRON {
	/**
	*	@brief Inherited class that handles wrapping render textures to be attached to frame buffers.
	*/
	class RenderTexture : public TextureWrapperBase {
	public:
		RenderTexture(unsigned int a_width = 1280, unsigned int a_height = 720);
		virtual ~RenderTexture();
	protected:
	private:
	};
}