#include "RendererProgram.h"
#include "Transform.h"
#include "RenderCamera.h"
#include "InputMonitor.h"
#include "Renderer_Utility_Funcs.h"
#include "Renderer_Utility_Literals.h"
#include "ShaderWrapper.h"
#include "VertexFormat.h"
#include "Mesh.h"
#include "Texture\Texture.h"
#include "Light\PhongLight_Dir.h"
#include "Light\PhongLight_Point.h"
#include "Light\PhongLight_Spot.h"
#include "Model.h"
#include "PostProcessing.h"

#include <AIE/Gizmos.h>
#include <glm/vec4.hpp>
#include <glm/ext.hpp>
#include <iostream>
#include <GLFW\glfw3.h>
#include <fstream>
#include <string>
#include <streambuf>
#include <imgui.h>
#include <stb/stb_image.h>
#include <gl_core_4_4.h>
#include <algorithm>

namespace SPRON {

	RendererProgram::RendererProgram()
	{
	}

	RendererProgram::~RendererProgram()
	{

	}

	int RendererProgram::Startup()
	{
		aie::Gizmos::create(10000, 10000, 10000, 10000);

		/// Variable initialisation
#pragma region Gizmos
		viewMatrix = glm::lookAt(glm::vec3(-10, 10, -10), glm::vec3(0), glm::vec3(0, 1, 0));			// Create view matrix looking at at an arbitrary point
		projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, 16 / 9.f, 0.1f, 1000.f);			// Define how objects should be drawn

		sphereTransform = new Transform(nullptr, glm::vec3(0, 2, 8), glm::vec3(3, 3, 3), glm::vec3(glm::radians(90.f), 0, 0));
#pragma endregion

		// Camera
		mainCamera = new RenderCamera();
		mainCamera->SetProjection(glm::radians(45.f), 16 / 9.f, 0.1f, 1000.f);
		mainCamera->GetTransform()->SetPosition(glm::vec3(0, 5, -5));
		mainCamera->GetTransform()->SetRotation(glm::vec3(glm::radians(20.f), 0, 0));

		/// Light initialisation
#pragma region Lights
#if ENABLE_DIR_LIGHTS
		sceneLights.push_back(new PhongLight_Dir(glm::vec4(0.f), glm::vec4(1.f), glm::vec4(1.f), glm::vec4(0, -1, -1, 0)));
		//sceneLights.push_back(new PhongLight_Dir(glm::vec4(0.f), glm::vec4(0.4f, 0.f, 0.f, 1.f), glm::vec4(0.5f), glm::vec4(1, 0, 0, 0)));
#endif

#if ENABLE_POINT_LIGHTS
		sceneLights.push_back(new PhongLight_Point(glm::vec4(0.f), glm::vec4(0.8f, 0.f, 0.f, 1.f), glm::vec4(1.f),
			DEFAULT_LIGHT_POS1, 20.f, DEFAULT_MIN_ILLUMINATION));
		sceneLights.push_back(new PhongLight_Point(glm::vec4(0.f), glm::vec4(1.f), glm::vec4(1),
			glm::vec4(0.f, 2.f, 0.f, 1.f), 10.f, DEFAULT_MIN_ILLUMINATION));
		//sceneLights.push_back(new PhongLight_Point(glm::vec4(0.f), glm::vec4(1.f), glm::vec4(1.f), glm::vec4(0.f, 12.5f, 4.f, 1.f), 200.f, DEFAULT_MIN_ILLUMINATION));
		//sceneLights.push_back(new PhongLight_Point(glm::vec4(0.f), glm::vec4(1.f), glm::vec4(1.f), glm::vec4(4.f, 12.5f, 4.f, 1.f), 50.f, DEFAULT_MIN_ILLUMINATION));
#endif

#if ENABLE_SPOT_LIGHTS
	sceneLights.push_back(new PhongLight_Spot(glm::vec4(0.f), glm::vec4(1.f), glm::vec4(1.f), 
		glm::vec4(-10.f, 0.f, 0.f, 1.f), glm::vec4(1, 0, 0, 0), 10.f, 14.f));
#endif
#pragma endregion

		/// Texture initialisation
#pragma region Textures
		faceTex = new Texture("./textures/awesomeface.png", "texture_diffuse", FILTERING_MIPMAP);

		wallTex = new Texture("./textures/wall.jpg", "texture_diffuse", FILTERING_MIPMAP);

		lightTex = new Texture("./textures/light.jpg", "texture_diffuse", FILTERING_MIPMAP);

		crateTex = new Texture("./textures/container2.png", "texture_diffuse", FILTERING_MIPMAP);

		crateSpecularTex = new Texture("./textures/container2_specular.png", "texture_specular", FILTERING_MIPMAP);

		floorTex = new Texture("./textures/wood_floor.jpg", "texture_diffuse", FILTERING_MIPMAP);
		floorTex->EnableWrapping();
#pragma endregion

		/// Shader initialisation
#pragma region Shaders
		//// Forward rendering shaders
		std::string forwardHeaderStr; RendererUtility::LoadTextToString("./shaders/phong/forward_header.glsl", forwardHeaderStr);

		ambientProgram = new ShaderWrapper();
		ambientProgram->LoadShader("./shaders/phong/forward_ambient.vert", VERT_SHADER);
		ambientProgram->LoadShader("./shaders/phong/forward_ambient.frag", FRAG_SHADER);
		ambientProgram->LinkShaders();

		directionalProgram = new ShaderWrapper();
		directionalProgram->LoadShader("./shaders/phong/forward_light.vert", VERT_SHADER);
		directionalProgram->LoadShader("./shaders/phong/forward_directional.frag", FRAG_SHADER, forwardHeaderStr.c_str());
		directionalProgram->LinkShaders();

		pointProgram = new ShaderWrapper();
		pointProgram->LoadShader("./shaders/phong/forward_light.vert", VERT_SHADER);
		pointProgram->LoadShader("./shaders/phong/forward_point.frag", FRAG_SHADER, forwardHeaderStr.c_str());
		pointProgram->LinkShaders();

		spotProgram = new ShaderWrapper();
		spotProgram->LoadShader("./shaders/phong/forward_light.vert", VERT_SHADER);
		spotProgram->LoadShader("./shaders/phong/forward_spot.frag", FRAG_SHADER, forwardHeaderStr.c_str());
		spotProgram->LinkShaders();

		// For debugging normals
		debugProgram = new ShaderWrapper();
		debugProgram->LoadShader("./shaders/debug/visualise_normals.vert", VERT_SHADER);
		debugProgram->LoadShader("./shaders/debug/visualise_normals.geom", GEOMETRY_SHADER);
		debugProgram->LoadShader("./shaders/debug/visualise_normals.frag", FRAG_SHADER);
		debugProgram->LinkShaders();

		//// Post-processing shaders
#if ENABLE_POST_PROCESSING
		PostProcessing::Activate();
#endif
		gammaEffect = new ShaderWrapper();
		gammaEffect->LoadShader("./shaders/post/post_base.vert", VERT_SHADER);
		gammaEffect->LoadShader("./shaders/post/post_gamma.frag", FRAG_SHADER);
		gammaEffect->LinkShaders();
#if ENABLE_POST_PROCESSING && ENABLE_GAMMA_CORRECT
		PostProcessing::AddEffect(gammaEffect);
#endif

		sharpenEffect = new ShaderWrapper();
		sharpenEffect->LoadShader("./shaders/post/post_base.vert", VERT_SHADER);
		sharpenEffect->LoadShader("./shaders/post/post_sharpen.frag", FRAG_SHADER);
		sharpenEffect->LinkShaders();

#if ENABLE_POST_PROCESSING && ENABLE_SHARPEN
		PostProcessing::AddEffect(sharpenEffect);
#endif

		blurEffect = new ShaderWrapper();
		blurEffect->LoadShader("./shaders/post/post_base.vert", VERT_SHADER);
		blurEffect->LoadShader("./shaders/post/post_blur.frag", FRAG_SHADER);
		blurEffect->LinkShaders();

#if ENABLE_POST_PROCESSING && ENABLE_BLUR
		PostProcessing::AddEffect(blurEffect);
#endif

		edgeDetectEffect = new ShaderWrapper();
		edgeDetectEffect->LoadShader("./shaders/post/post_base.vert", VERT_SHADER);
		edgeDetectEffect->LoadShader("./shaders/post/post_edge.frag", FRAG_SHADER);
		edgeDetectEffect->LinkShaders();

#if ENABLE_POST_PROCESSING && ENABLE_EDGE_DETECT
		PostProcessing::AddEffect(edgeDetectEffect);
#endif

#pragma endregion

		/// Material initialisation
#pragma region Materials
		Material crateMat;
		crateMat.ambientColor = glm::vec4(1);
		crateMat.diffuseColor = glm::vec4(1);
		crateMat.specular = glm::vec4(1);
		crateMat.shininessCoefficient = 200.f;
		crateMat.diffuseMap = crateTex;
		crateMat.specularMap = nullptr;

		Material planeMat;
		planeMat.ambientColor = glm::vec4(1);
		planeMat.diffuseColor = glm::vec4(1);
		planeMat.specular = glm::vec4(1);
		planeMat.shininessCoefficient = 32.f;
		planeMat.diffuseMap = floorTex;
#pragma endregion

		/// Mesh initialisation
#pragma region Models/VertFormats/Meshes
		// Models
		Model* floorModel = new Model("./models/floor/Sci-Fi-Floor-1-BLEND.obj"); floorModel->GetTransform()->SetScale(glm::vec3(10.f, 10.f, 10.f));
		sceneModels.push_back(floorModel);

		Model* devilModel = new Model("./models/theatre_devil/BIO-I_PC_N.P.C_Theatre_Devil.obj"); devilModel->GetTransform()->Translate(glm::vec3(10, 0, 0));
		sceneModels.push_back(devilModel);

		Model* clarissaModel = new Model("./models/clarissa/Clarissa.obj"); clarissaModel->GetTransform()->SetScale(glm::vec3(0.12, 0.12, 0.12));
		sceneModels.push_back(clarissaModel);

		Model* skullModel = new Model("./models/skull/Skull.obj"); skullModel->GetTransform()->Translate(glm::vec3(0, 0, 15)); skullModel->GetTransform()->SetScale(glm::vec3(0.01, 0.01, 0.01));
		sceneModels.push_back(skullModel);

		// Vertex formats
		VertexFormat* rectFormat = new VertexFormat(std::vector<unsigned int>{
			0, 1, 2,	// First triangle
				1, 2, 3		// Second triangle
		});
		VertexFormat* cubeFormat = new VertexFormat(std::vector<unsigned int>{0});

		// Meshes
		//// Cube
		std::vector<Vertex> cubeVerts = {
				Vertex(glm::vec4(-0.5f, -0.5f, -0.5f, 1.f), glm::vec2(0.f, 0.f), glm::vec4(0.0f,  0.0f, -1.0f, 0.f)),
				Vertex(glm::vec4(0.5f, -0.5f, -0.5f, 1.f),	glm::vec2(1.f, 0.f), glm::vec4(0.0f,  0.0f, -1.0f, 0.f)),
				Vertex(glm::vec4(0.5f,  0.5f, -0.5f, 1.f),	glm::vec2(1.f, 1.f), glm::vec4(0.0f,  0.0f, -1.0f, 0.f)),
				Vertex(glm::vec4(0.5f,  0.5f, -0.5f, 1.f),	glm::vec2(1.f, 1.f), glm::vec4(0.0f,  0.0f, -1.0f, 0.f)),
				Vertex(glm::vec4(-0.5f,  0.5f, -0.5f,1.f),	glm::vec2(0.f, 1.f), glm::vec4(0.0f,  0.0f, -1.0f, 0.f)),
				Vertex(glm::vec4(-0.5f, -0.5f, -0.5f,1.f),	glm::vec2(0.f, 0.f), glm::vec4(0.0f,  0.0f, -1.0f, 0.f)),

				Vertex(glm::vec4(-0.5f, -0.5f,  0.5f,1.f),	glm::vec2(0.f, 0.f), glm::vec4(0.0f,  0.0f, 1.0f,0.f)),
				Vertex(glm::vec4(0.5f, -0.5f,  0.5f, 1.f),	glm::vec2(1.f, 0.f), glm::vec4(0.0f,  0.0f, 1.0f,0.f)),
				Vertex(glm::vec4(0.5f,  0.5f,  0.5f, 1.f),	glm::vec2(1.f, 1.f), glm::vec4(0.0f,  0.0f, 1.0f,0.f)),
				Vertex(glm::vec4(0.5f,  0.5f,  0.5f, 1.f),	glm::vec2(1.f, 1.f), glm::vec4(0.0f,  0.0f, 1.0f,0.f)),
				Vertex(glm::vec4(-0.5f,  0.5f,  0.5f,1.f),	glm::vec2(0.f, 1.f), glm::vec4(0.0f,  0.0f, 1.0f,0.f)),
				Vertex(glm::vec4(-0.5f, -0.5f,  0.5f,1.f),	glm::vec2(0.f, 0.f), glm::vec4(0.0f,  0.0f, 1.0f,0.f)),

				Vertex(glm::vec4(-0.5f,  0.5f,  0.5f,1.f),	glm::vec2(1.f, 0.f), glm::vec4(-1.0f,  0.0f,  0.0f, 0.f)),
				Vertex(glm::vec4(-0.5f,  0.5f, -0.5f,1.f),	glm::vec2(1.f, 1.f), glm::vec4(-1.0f,  0.0f,  0.0f, 0.f)),
				Vertex(glm::vec4(-0.5f, -0.5f, -0.5f,1.f),	glm::vec2(0.f, 1.f), glm::vec4(-1.0f,  0.0f,  0.0f, 0.f)),
				Vertex(glm::vec4(-0.5f, -0.5f, -0.5f,1.f),	glm::vec2(0.f, 1.f), glm::vec4(-1.0f,  0.0f,  0.0f, 0.f)),
				Vertex(glm::vec4(-0.5f, -0.5f,  0.5f,1.f),	glm::vec2(0.f, 0.f), glm::vec4(-1.0f,  0.0f,  0.0f, 0.f)),
				Vertex(glm::vec4(-0.5f,  0.5f,  0.5f,1.f),	glm::vec2(1.f, 0.f), glm::vec4(-1.0f,  0.0f,  0.0f, 0.f)),

				Vertex(glm::vec4(0.5f,  0.5f,  0.5f, 1.f),	glm::vec2(1.f, 0.f), glm::vec4(1.0f,  0.0f,  0.0f,	 0.f)),
				Vertex(glm::vec4(0.5f,  0.5f, -0.5f, 1.f),	glm::vec2(1.f, 1.f), glm::vec4(1.0f,  0.0f,  0.0f,	 0.f)),
				Vertex(glm::vec4(0.5f, -0.5f, -0.5f, 1.f),	glm::vec2(0.f, 1.f), glm::vec4(1.0f,  0.0f,  0.0f,	 0.f)),
				Vertex(glm::vec4(0.5f, -0.5f, -0.5f, 1.f),	glm::vec2(0.f, 1.f), glm::vec4(1.0f,  0.0f,  0.0f,	 0.f)),
				Vertex(glm::vec4(0.5f, -0.5f,  0.5f, 1.f),	glm::vec2(0.f, 0.f), glm::vec4(1.0f,  0.0f,  0.0f,	 0.f)),
				Vertex(glm::vec4(0.5f,  0.5f,  0.5f, 1.f),	glm::vec2(1.f, 0.f), glm::vec4(1.0f,  0.0f,  0.0f,	 0.f)),

				Vertex(glm::vec4(-0.5f, -0.5f, -0.5f,1.f),  glm::vec2(0.f, 1.f), glm::vec4(0.0f, -1.0f,  0.0f, 0.f)),
				Vertex(glm::vec4(0.5f, -0.5f, -0.5f, 1.f),	glm::vec2(1.f, 1.f), glm::vec4(0.0f, -1.0f,  0.0f,	 0.f)),
				Vertex(glm::vec4(0.5f, -0.5f,  0.5f, 1.f),	glm::vec2(1.f, 0.f), glm::vec4(0.0f, -1.0f,  0.0f,	 0.f)),
				Vertex(glm::vec4(0.5f, -0.5f,  0.5f, 1.f),	glm::vec2(1.f, 0.f), glm::vec4(0.0f, -1.0f,  0.0f,	 0.f)),
				Vertex(glm::vec4(-0.5f, -0.5f,  0.5f,1.f),	glm::vec2(0.f, 0.f), glm::vec4(0.0f, -1.0f,  0.0f, 0.f)),
				Vertex(glm::vec4(-0.5f, -0.5f, -0.5f,1.f),	glm::vec2(0.f, 1.f), glm::vec4(0.0f, -1.0f,  0.0f, 0.f)),

				Vertex(glm::vec4(-0.5f,  0.5f, -0.5f,1.f),	glm::vec2(0.f, 1.f), glm::vec4(0.0f,  1.0f,  0.0f, 0.f)),
				Vertex(glm::vec4(0.5f,  0.5f, -0.5f, 1.f),	glm::vec2(1.f, 1.f), glm::vec4(0.0f,  1.0f,  0.0f,	 0.f)),
				Vertex(glm::vec4(0.5f,  0.5f,  0.5f, 1.f),	glm::vec2(1.f, 0.f), glm::vec4(0.0f,  1.0f,  0.0f,	 0.f)),
				Vertex(glm::vec4(0.5f,  0.5f,  0.5f, 1.f),	glm::vec2(1.f, 0.f), glm::vec4(0.0f,  1.0f,  0.0f,	 0.f)),
				Vertex(glm::vec4(-0.5f,  0.5f,  0.5f,1.f),	glm::vec2(0.f, 0.f), glm::vec4(0.0f,  1.0f,  0.0f, 0.f)),
				Vertex(glm::vec4(-0.5f,  0.5f, -0.5f,1.f),	glm::vec2(0.f, 1.f), glm::vec4(0.0f,  1.0f,  0.0f, 0.f))
		};

		for (int i = 0; i < DEFAULT_CUBE_NUM; ++i) {
			sceneMeshes.push_back(new Mesh(cubeVerts, cubeFormat, new Transform(), crateMat));

			sceneMeshes[i]->GetTransform()->SetPosition(glm::vec3(i * 2, i * 2, i * 2));
			sceneMeshes[i]->GetTransform()->SetScale(glm::vec3(1));
		}
#pragma endregion

		/// Draw mode
#if DRAW_WIREFRAME
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);		// Wireframe applied to front and back of triangles
#endif

		return 0;
	}

	void RendererProgram::Shutdown()
	{
		aie::Gizmos::destroy();

		delete wallTex;
		delete faceTex;
		delete lightTex;
		delete crateTex;
		delete crateSpecularTex;
		delete floorTex;

		delete sphereTransform;

		delete mainCamera;

		for (int i = 0; i < sceneMeshes.size(); ++i) {
			delete sceneMeshes[i];
		}
		sceneMeshes.clear();

		for (int i = 0; i < sceneModels.size(); ++i) {
			delete sceneModels[i];
		}
		sceneModels.clear();

		for (int i = 0; i < sceneLights.size(); ++i) {
			delete sceneLights[i];
		}
		sceneLights.clear();

		delete ambientProgram;
		delete directionalProgram;
		delete spotProgram;
		delete pointProgram;
		delete debugProgram;
		delete gammaEffect;
		delete sharpenEffect;
		delete blurEffect;
		delete edgeDetectEffect;
	}

	void RendererProgram::FixedUpdate(float a_dt)
	{
		static float m_accumulatedTime = 0.f;
		static float m_fixedTimeStep = 0.01f;

		m_accumulatedTime += a_dt;

		// Run update until no more extra time between updates left
		while (m_accumulatedTime >= m_fixedTimeStep) {
			InputMonitor* input = InputMonitor::GetInstance();

			mainCamera->Update(m_fixedTimeStep);

			/// Lighting update
			for (int i = 0; i < sceneLights.size(); ++i) {
				// Update spot light position and direction based off camera
				if (sceneLights[i]->GetType() == SPOT_LIGHT) {
					PhongLight_Spot* spotLight = (PhongLight_Spot*)sceneLights[i];

					spotLight->SetPos(glm::vec4(mainCamera->GetTransform()->GetPosition(), 1));
					spotLight->SetSpotDir(glm::vec4(mainCamera->GetTransform()->Forward(), 0));
				}

				// Orbit point lights
				if (sceneLights[i]->GetType() == POINT_LIGHT) {

					PhongLight_Point* ptLight = (PhongLight_Point*)sceneLights[i];

					static float orbitRadius = 2.f;
					static float orbitHeight = 10.f;
					static float scale = 1.f;

					glm::vec4 orbitVec = glm::vec4(cosf(glfwGetTime()) * orbitRadius * scale, orbitHeight, sinf(glfwGetTime()) * orbitRadius * scale, 0.f);
				}
			}

			// Turn flash light on and off
			if (input->GetKeyDown(GLFW_KEY_X)) {		// Toggle flashlight
				isFlashLightOn = !isFlashLightOn;
			}

			// Rotate models
			static float rotSpeed = 10.f;

			for (int i = 0; i < sceneModels.size(); ++i) {
				sceneModels[i]->GetTransform()->SetRotation(glm::vec3(0, glm::radians(glfwGetTime()) * rotSpeed, 0));
				//sceneModels[i]->GetTransform()->SetPosition(glm::vec4(glfwGetTime(), 0, 0, 0));
			}

			m_accumulatedTime -= m_fixedTimeStep;	// Account for time overflow
		}
	}


	void RendererProgram::Update(float a_dt)
	{
		FixedUpdate(a_dt);

#pragma region Gizmos
		aie::Gizmos::clear();									// Refresh gizmos for new frame

		aie::Gizmos::addTransform(glm::mat4(1));				// Visually represent identity matrix with unit vectors
		aie::Gizmos::addTransform(mainCamera->GetTransform()->GetMatrix());

		aie::Gizmos::addLine(mainCamera->GetTransform()->GetPosition(), mainCamera->GetTransform()->GetPosition() + (mainCamera->GetTransform()->Forward() * 2), glm::vec4(1, 1, 0, 1));
		aie::Gizmos::addLine(mainCamera->GetTransform()->GetPosition(), mainCamera->GetTransform()->GetPosition() + (mainCamera->GetTransform()->Up() * 2), glm::vec4(0, 1, 1, 1));


		glm::vec4 white(1);
		glm::vec4 black(0, 0, 0, 1);

		// 2D grid
		for (int i = 0; i < 21; ++i) {
			// Vertical lines
			aie::Gizmos::addLine(
				glm::vec3(i + -10, 0, 10),
				glm::vec3(i + -10, 0, -10),
				i == 10 ? white : black			// Every grid line except middle is black
			);

			// Horizontal lines
			aie::Gizmos::addLine(
				glm::vec3(10, 0, i + -10),
				glm::vec3(-10, 0, i + -10),
				i == 10 ? white : black
			);
		}

		// Point lights
		for (int i = 0; i < sceneLights.size(); ++i) {
			if (sceneLights[i]->GetType() == POINT_LIGHT) {
				PhongLight_Point* ptLight = (PhongLight_Point*)sceneLights[i];

				glm::vec4 lightColor = ptLight->GetAmbient(); lightColor.w = 0.5f;
				glm::vec4 radiusColor = ptLight->GetDiffuse(); radiusColor.w = 0.2f;

				aie::Gizmos::addSphere(ptLight->GetPos(), 0.1, 12, 12, radiusColor);									// Light
#ifdef DEBUG
				float radiusScale = 1.f;
				aie::Gizmos::addSphere(ptLight->GetPos(), ptLight->GetIlluminationRadius() * radiusScale, 12, 12, radiusColor);		// Illumination radius
#endif
			}
		}

#pragma endregion Gizmo creation

#if ENABLE_POST_PROCESSING && ENABLE_SHARPEN
		ImGui::Begin("Sharpen Values");
		static float in_sharpenClarity = 100.f; ImGui::InputFloat("Clarity Factor", &in_sharpenClarity, 25.f); sharpenEffect->SetFloat("clarityFactor", in_sharpenClarity);
		ImGui::End();
#endif

#if ENABLE_POST_PROCESSING && ENABLE_BLUR
		ImGui::Begin("Blur Values");
		static float in_blurClarity = 100.f; ImGui::InputFloat("Clarity Factor", &in_blurClarity, 25.f); blurEffect->SetFloat("clarityFactor", in_blurClarity);
		ImGui::End();
#endif

#if ENABLE_POST_PROCESSING && ENABLE_EDGE_DETECT
		ImGui::Begin("Edge Detect Values");
		static float in_edgeDetectClarity = 100.f; ImGui::InputFloat("Clarity Factor", &in_edgeDetectClarity, 25.f); edgeDetectEffect->SetFloat("clarityFactor", in_edgeDetectClarity);
		ImGui::End();
#endif

		/// Light properties
		for (int i = 0; i < sceneLights.size(); ++i) {
			
			PhongLight_Point* ptLight = (sceneLights[i]->GetType() == POINT_LIGHT) ? (PhongLight_Point*)sceneLights[i] : nullptr;

			if (ptLight) {
				ptLight->ListenIMGUI();
			}
		}

		/// Material properties
		for (int i = 0; i < sceneModels.size(); ++i) {
			auto meshes = sceneModels[i]->GetModelMeshes();

			ImGui::Begin(sceneModels[i]->GetDirectory().c_str());

			for (int j = 0; j < meshes.size(); ++j) {
				Material& currMaterial = meshes[j]->GetMaterial();

				ImGui::PushID(j);		// Give material block unique identifier so multiple materials with the same properties can exist
				meshes[j]->GetMaterial().ListenIMGUI();
				ImGui::PopID();
			}

			ImGui::End();

		}

	}

	void RendererProgram::Render()
	{
#if ENABLE_POST_PROCESSING
		PostProcessing::BeginListening();
#endif

#if DRAW_GIZMOS
#if USE_GIZMO_CAMERA
		aie::Gizmos::draw(projectionMatrix * viewMatrix);
#else
		aie::Gizmos::draw(mainCamera->CalculateProjectionView());
#endif
#endif

		ShaderWrapper* flashLight = (isFlashLightOn ? spotProgram : nullptr);	// Ignore spot light program pass if flash light isn't on

		// Meshes
		ShaderWrapper* normalDraw = nullptr;
#if DRAW_NORMALS
		normalDraw = debugProgram;
#endif

		for (int i = 0; i < sceneMeshes.size(); ++i) {
			sceneMeshes[i]->Draw(mainCamera, sceneLights, globalAmbient, ambientProgram, directionalProgram, pointProgram, flashLight, normalDraw);
		}

		// Models
		for (int i = 0; i < sceneModels.size(); ++i) {
			sceneModels[i]->Draw(mainCamera, sceneLights, globalAmbient, ambientProgram, directionalProgram, pointProgram, flashLight, normalDraw);
		}

		// Post-processing
#if ENABLE_POST_PROCESSING
		PostProcessing::Draw();
#endif
	}
}