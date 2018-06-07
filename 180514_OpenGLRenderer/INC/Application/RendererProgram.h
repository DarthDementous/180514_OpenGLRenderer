#pragma once

#include <Program.h>
#include <glm/mat4x4.hpp>
#include <vector>

namespace SPRON {
	class Mesh;
	class Model;
	class RenderCamera;
	class Transform;
	class Texture;
	class PhongLight;
	class ShaderWrapper;
}

namespace SPRON {
	class RendererProgram : public Program {
	public:
		RendererProgram();
		virtual ~RendererProgram();
	protected:
		virtual int		Startup();
		virtual void	Shutdown();

		virtual void Update(float a_dt);
		virtual void Render();
	private:
		void FixedUpdate(float a_dt);

		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;

		RenderCamera* mainCamera;

		Transform* sphereTransform;

		std::vector<Mesh*> sceneMeshes;

		Model* nanosuitModel;
		Model* floorModel;

		Texture* wallTex;
		Texture* faceTex;
		Texture* lightTex;
		Texture*	crateTex;
		Texture* crateSpecularTex;
		Texture* floorTex;

		/// Forward rendering
		glm::vec4 globalAmbient = glm::vec4(0.01, 0.01, 0.01, 1.f);

		// Shader programs for respective light type calculations
		ShaderWrapper* ambientProgram;
		ShaderWrapper* directionalProgram;
		ShaderWrapper* pointProgram;
		ShaderWrapper* spotProgram;

		// Shader programs for post-processing
		ShaderWrapper* gammaEffect;
		ShaderWrapper* sharpenEffect;
		ShaderWrapper* blurEffect;
		ShaderWrapper* edgeDetectEffect;

		std::vector<PhongLight*> sceneLights;

		bool isFlashLightOn = true;
	};
}