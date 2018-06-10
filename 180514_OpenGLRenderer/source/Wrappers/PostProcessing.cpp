#include "PostProcessing.h"
#include "Texture/RenderTexture.h"
#include "VertexFormat.h"
#include "Vertex.h"
#include "Mesh.h"
#include "Transform.h"
#include "Renderer_Utility_Literals.h"
#include "ShaderWrapper.h"

#include <gl_core_4_4.h>
#include <iostream>
#include <imgui.h>

namespace SPRON {
	/// Static initialisation
	PostProcessing* PostProcessing::m_stn = nullptr;

	/**
	*	@brief Initialise singleton and create and bind frame buffer to 'hijack' rendering calls with a render texture and render buffer attached.
	*	NOTE: Any future active calls will be ignored.
	*/
	void PostProcessing::Activate()
	{
		if (!m_stn) {		// Post processing hasn't been activated yet

			m_stn = new PostProcessing();

			// Create and bind frame buffer
			glGenFramebuffers(1, &m_stn->m_frameBufferID);

			glBindFramebuffer(GL_FRAMEBUFFER, m_stn->m_frameBufferID);	// Allow future read and write frame buffer operations to affect this buffer
			
			/// Create and attach render texture to frame buffer [SCREEN TEXTURE]
			GLint viewport[4]; glGetIntegerv(GL_VIEWPORT, viewport); int screenWidth = viewport[2], screenHeight = viewport[3];

			m_stn->m_screenTex = new RenderTexture(screenWidth, screenHeight);	// Make sure texture covers frame dimensions

			glFramebufferTexture2D(
				GL_FRAMEBUFFER,				// Frame buffer target 
				GL_COLOR_ATTACHMENT0,		// Type of attachment (can be multiple)
				GL_TEXTURE_2D,				// Type of texture being attached
				*m_stn->m_screenTex,		// Texture ID
				0);							// Mipmap level (kept at 0 because not being used) 

			/// Create and attach render buffer to frame buffer
			glGenRenderbuffers(1, &m_stn->m_renderBufferID);

			glBindRenderbuffer(GL_RENDERBUFFER, m_stn->m_renderBufferID);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth, screenHeight);	// Track depth with render buffer in order to perform depth testing so only the closest pixels render, avoiding overlap
			glBindRenderbuffer(GL_RENDERBUFFER, 0);													// No need for render buffer to be bound after setting storage

			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_stn->m_renderBufferID);

			// Check if frame buffer is 'complete' (at least one buffer attached, at least one color attachment, same number of samples - complete attachments)
			try {
				if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {	// Frame buffer is incomplete
					char errorMsg[256];
					sprintf_s(errorMsg, "ERROR::FRAME_BUFFER::INCOMPLETE: %s");

					throw std::runtime_error(errorMsg);
				}
			}
			catch (std::exception const& e) { std::cout << "Exception: " << e.what() << std::endl; }

			// Create quad mesh for screen
			std::vector<Vertex> quadVerts = {
				Vertex(glm::vec4(1.f, 1.f, 0.f, 1.f), glm::vec2(1, 1), glm::vec4()),	// Top right
				Vertex(glm::vec4(1.f, -1.f, 0.f, 1.f), glm::vec2(1, 0), glm::vec4()),	// Bottom right
				Vertex(glm::vec4(-1.f, -1.f, 0.f, 1.f), glm::vec2(0, 0), glm::vec4()),	// Bottom left
				Vertex(glm::vec4(-1.f, 1.f, 0.f, 1.f), glm::vec2(0, 1), glm::vec4())	// Top left
			};

			m_stn->m_quadFormat = new VertexFormat(std::vector<unsigned int> {
				0, 1, 3,     // First triangle
				1, 2, 3		 // Second triangle
			});

			m_stn->m_screenMesh = new Mesh(quadVerts, m_stn->m_quadFormat, new Transform());

			// Initialise default screen shader
			m_stn->m_baseEffect = new ShaderWrapper();
			m_stn->m_baseEffect->LoadShader("./shaders/post/post_base.vert", VERT_SHADER);
			m_stn->m_baseEffect->LoadShader("./shaders/post/post_hdr_bloom.frag", FRAG_SHADER);
			m_stn->m_baseEffect->LinkShaders();

			m_stn->m_baseEffect->SetTexture("screenRenderTex", m_stn->m_screenTex);

			// Unbind custom frame buffer
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}

	/**
	*	@brief Add a post-processing shader program to the effects vector to be processed in the draw function.
	*	NOTE: Can't be used unless post processing has been activated or else will trigger null exception.
	*	@param a_effectProgram is the shader program to add.
	*	@return void.
	*/
	void PostProcessing::AddEffect(ShaderWrapper* a_effectProgram)
	{
		m_stn->m_effects.push_back(a_effectProgram);
	}

	/**
	*	@brief Bind custom frame buffer so it receives rendering information that will then later be outputted by the draw function.
	*	@return void.
	*/
	void PostProcessing::BeginListening()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_stn->m_frameBufferID);
		glEnable(GL_DEPTH_TEST);			// Enable depth testing to accurately replace the default frame buffer

		// Refresh previous frame buffer content
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	/**
	*	@brief Render a pass on the screen texture mesh for each added effect.
	*	NOTE: Post-processing must be activated and it MUST be called AFTER BeginListening.
	*	@return void.
	*/
	void PostProcessing::Draw()
	{
		// Bind back to default frame buffer to enable rendering to the window instead of off-screen rendering in the custom frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);		// Disable depth test or the screen quad will be discarded
		glClear(GL_COLOR_BUFFER_BIT);	// No depth-testing, only need to refresh color

		// Optional HDR
		ImGui::Begin("HDR");
		
		static bool in_enableHDR = true; ImGui::Checkbox("Enable HDR", &in_enableHDR); m_stn->m_baseEffect->SetBool("enableHDR", in_enableHDR);
		if (in_enableHDR) {
			static float in_exposure = 1; ImGui::DragFloat("Exposure", &in_exposure, 0.01f, 0.f, 10.f); m_stn->m_baseEffect->SetFloat("exposure", in_exposure);
		}

		ImGui::End();

		// Draw screen render texture
		m_stn->m_screenMesh->Render(m_stn->m_baseEffect);

#if BLEND_POST_PROCESSING
		// Activate blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);	// Take existing frag color *1 and add it onto the new frag color *1

		/// Post-processing effect passes
		ImGui::Begin("Post Processing Effects");

#if ENABLE_SHARPEN
		static bool in_enableSharpen = false; ImGui::Checkbox("Enable Sharpen", &in_enableSharpen);
		static float in_sharpenClarity = 100.f; if (in_enableSharpen) ImGui::DragFloat("Sharpen Clarity", &in_sharpenClarity, 1.f);
#endif

#if ENABLE_BLUR
		static bool in_enableBlur = false; ImGui::Checkbox("Enable Blur", &in_enableBlur);
		static float in_blurClarity = 100.f; if (in_enableBlur) ImGui::DragFloat("Blur Clarity", &in_blurClarity, 1.f);
#endif

#if ENABLE_EDGE_DETECT
		static bool in_enableEdge = false; ImGui::Checkbox("Enable Edge", &in_enableEdge);
		static float in_edgeClarity = 100.f; if (in_enableEdge) ImGui::DragFloat("Edge Clarity", &in_edgeClarity, 1.f); 
#endif
		ImGui::End();

		for (int i = 0; i < m_stn->m_effects.size(); ++i) {
			ShaderWrapper* currentEffect = m_stn->m_effects[i];

			// Set render texture parameter
			currentEffect->SetTexture("screenRenderTex", m_stn->m_screenTex);

			/// Sharpen
#if ENABLE_SHARPEN
			if (in_enableSharpen && currentEffect->GetName() == "post_sharpen") { 
				
				currentEffect->SetFloat("clarityFactor", in_sharpenClarity);
				m_stn->m_screenMesh->Render(currentEffect);
			}
#endif

			/// Blur
#if ENABLE_BLUR
			if (in_enableBlur && currentEffect->GetName() == "post_blur") {
				
				currentEffect->SetFloat("clarityFactor", in_blurClarity);
				m_stn->m_screenMesh->Render(currentEffect);
			}
#endif
			/// Edge detect
#if ENABLE_EDGE_DETECT
			if (in_enableEdge && currentEffect->GetName() == "post_edge") {

				currentEffect->SetFloat("clarityFactor", in_edgeClarity);
				m_stn->m_screenMesh->Render(currentEffect);
			}
#endif
		}

		// De-activate blending
		glDisable(GL_BLEND);
#endif
	}

	PostProcessing::PostProcessing()
	{
	}

	PostProcessing::~PostProcessing()
	{
		// Clean up unique variable memory
		delete m_stn->m_quadFormat;
		delete m_stn->m_screenMesh;
		
		delete m_stn->m_baseEffect;

		// Clean up openGL frame buffer object
		glDeleteFramebuffers(1, &m_stn->m_frameBufferID);

		// Clean up openGL render buffer object
		glDeleteRenderbuffers(1, &m_stn->m_frameBufferID);

		// Clean up render texture
		delete m_stn->m_screenTex;

		// Destroy singleton (NOTE: Must come last!)
		delete m_stn;
	}
}