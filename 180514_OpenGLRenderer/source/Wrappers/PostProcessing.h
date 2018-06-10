#pragma once

#include <vector>

namespace SPRON {
	class RenderTexture;
	class ShaderWrapper;
	class Mesh;
	class VertexFormat;

	/**
	*	@brief Static singleton class that handles rendering multiple passes of post-processing effects to a frame buffer.
	*/
	class PostProcessing {
	public:
		static void Activate();
		static void AddEffect(ShaderWrapper* a_effectProgram);
		static void BeginListening();
		static void Draw();

		static unsigned int GetFrameBufferID() { return m_stn->m_frameBufferID; }
	protected:
	private:
		static PostProcessing* m_stn;		// Singleton instance

		// Instance variables
		unsigned int	m_frameBufferID;

		unsigned int	m_renderBufferID;
		RenderTexture*	m_screenTex;

		VertexFormat*	m_quadFormat;				// Hold onto quad format so memory can be cleaned up
		Mesh*			m_screenMesh;				// Mesh used for drawing over screen with default frame buffer

		std::vector<ShaderWrapper*>	m_effects;		// Keep vector of shader programs to use for post-processing render passes
		ShaderWrapper*	m_baseEffect;				// Default rendering effect (usually just draws render texture untouched)

		PostProcessing();
		~PostProcessing();
	};
}