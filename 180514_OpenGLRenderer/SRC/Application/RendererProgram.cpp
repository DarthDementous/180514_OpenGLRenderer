#include "RendererProgram.h"
#include "Transform.h"
#include "RenderCamera.h"
#include "InputMonitor.h"
#include "Renderer_Utility_Funcs.h"
#include "Renderer_Utility_Literals.h"
#include "ShaderWrapper.h"
#include "VertexFormat.h"
#include "Mesh.h"
#include "TextureWrapper.h"
#include "Light\PhongLight_Dir.h"
#include "Light\PhongLight_Point.h"
#include "Light\PhongLight_Spot.h"
#include "Model.h"

#include <AIE/Gizmos.h>
#include <glm/vec4.hpp>
#include <glm/ext.hpp>
#include <iostream>
#include <GLFW\glfw3.h>
#include <fstream>
#include <string>
#include <streambuf>

#include <stb/stb_image.h>

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
	viewMatrix = glm::lookAt(glm::vec3(-10, 10, -10), glm::vec3(0), glm::vec3(0, 1, 0));			// Create view matrix looking at at an arbitrary point
	projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, 16 / 9.f, 0.1f, 1000.f);			// Define how objects should be drawn

	sphereTransform = new Transform(glm::vec3(0, 2, 8), glm::vec3(3, 3, 3), glm::vec3(glm::radians(90.f), 0, 0));

	mainCamera = new RenderCamera();
	mainCamera->SetProjection(glm::radians(45.f), 16 / 9.f, 0.1f, 1000.f);
	mainCamera->GetTransform()->SetPosition(glm::vec3(0, 5, -5));
	mainCamera->GetTransform()->SetRotation(glm::vec3(glm::radians(20.f), 0, 0));

	/// Light initialisation
	sceneLights.push_back(new PhongLight_Dir(glm::vec4(0.f), glm::vec4(255.f / 255, 215.f / 255, 0.f, 0.5f) * 0.25f, glm::vec4(255.f / 255, 215.f / 255, 0.f, 0.1f), DEFAULT_LIGHT_DIR));	// Gold specular highlights
	sceneLights.push_back(new PhongLight_Dir(glm::vec4(0.f), glm::vec4(0.4f, 0.f, 0.f, 1.f), glm::vec4(0.5f), glm::vec4(1, 0, 0, 0)));

	sceneLights.push_back(new PhongLight_Point(glm::vec4(0.f), glm::vec4(0.8f, 0.f, 0.f, 1.f), glm::vec4(1.f), 
		DEFAULT_LIGHT_POS1, 20.f, DEFAULT_MIN_ILLUMINATION));
	sceneLights.push_back(new PhongLight_Point(glm::vec4(0.f), glm::vec4(0.f, 0.8f, 0.f, 1.f), glm::vec4(1.f), 
		DEFAULT_LIGHT_POS2, 20.f, DEFAULT_MIN_ILLUMINATION));

	sceneLights.push_back(new PhongLight_Spot(glm::vec4(0.05f), glm::vec4(1.f), glm::vec4(1.f), 
		glm::vec4(-10.f, 0.f, 0.f, 1.f), glm::vec4(1, 0, 0, 0), 10.f, 14.f));

	/// Texture initialisation
	if (WRAPPED_OGL_TEX) {
		faceTex = new TextureWrapper("./textures/awesomeface.png", "texture_diffuse");
		faceTex->EnableFiltering();
		faceTex->EnableMipmapping();
		faceTex->EnableWrapping();

		wallTex = new TextureWrapper("./textures/wall.jpg", "texture_diffuse");
		wallTex->EnableFiltering();
		wallTex->EnableMipmapping();
		wallTex->EnableWrapping();

		lightTex = new TextureWrapper("./textures/light.jpg", "texture_diffuse");
		lightTex->EnableFiltering();
		lightTex->EnableMipmapping();
		lightTex->EnableWrapping();

		crateTex = new TextureWrapper("./textures/container2.png", "texture_diffuse");
		crateTex->EnableFiltering();
		crateTex->EnableMipmapping();
		crateTex->EnableWrapping();

		crateSpecularTex = new TextureWrapper("./textures/container2_specular.png", "texture_specular");
		crateSpecularTex->EnableFiltering();
		crateSpecularTex->EnableMipmapping();
		crateSpecularTex->EnableWrapping();
	}
	else {
		// Load wall texture data into char array
		int texWidth, texHeight, channelNum;
		unsigned char* texData = stbi_load("./textures/wall.jpg", &texWidth, &texHeight, &channelNum, 0);

		try {
			if (!texData) {
				char errorMsg[256];
				sprintf_s(errorMsg, "ERROR::IMAGE::FAILED_TO_LOAD: %s", "./textures/wall.jpg");

				throw std::runtime_error(errorMsg);
			}
		}
		catch (std::exception const& e) { std::cout << "Exception: " << e.what() << std::endl; }

		// Create texture on GPU
		unsigned int textureKernel;
		glGenTextures(1, &textureKernel);

		// Bind texture and set texture units
		glActiveTexture(GL_TEXTURE1);						// NOTE: Texture unit 0 is activated by default	
		glBindTexture(GL_TEXTURE_2D, textureKernel);		// Upon calling, texture is bound to the currently active texture unit

		// Wrapping
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);		// 2D Texture wrap mode on x axis
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);		// 2D Texture wrap mode on y axis

		// Filtering	(GL_NEAREST = take color of pixel whose center is closest to texture coord, GL_LINEAR = get average color from neighboring pixels to texture coord)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);			// Filtering mode for scaling down textures
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);			// Filtering mode for scaling up textures (Bilinear)
																							// Set texture data and generate mipmaps
		glTexImage2D(	// NOTE: Applies to currently bound texture
			GL_TEXTURE_2D,		// Enum for texture dimension
			0,					// Mipmap level (0 by default)
			GL_RGB,				// Format to STORE texture in
			texWidth,			// Width of texture (pixels)
			texHeight,			// Height of texture (pixels)
			0,					// Legacy parameter, must be 0
			GL_RGB,				// Format of SOURCE texture
			GL_UNSIGNED_BYTE,	// Type of data in source texture
			texData);			// Memory location of texture data

		glGenerateMipmap(GL_TEXTURE_2D);	// Auto-generate mipmaps for bound texture

											// Free image source memory after transferral to GPU
		stbi_image_free(texData);

	}

	if (WRAPPED_OGL_OTHER) {
		/// Shader initialisation
		//// Forward rendering shaders
		ambientProgram = new ShaderWrapper();
		ambientProgram->LoadShader("./shaders/forward_ambient.vert", VERT_SHADER);
		ambientProgram->LoadShader("./shaders/forward_ambient.frag", FRAG_SHADER);
		ambientProgram->LinkShaders();

		directionalProgram = new ShaderWrapper();
		directionalProgram->LoadShader("./shaders/forward_directional.vert", VERT_SHADER);
		directionalProgram->LoadShader("./shaders/forward_directional.frag", FRAG_SHADER);
		directionalProgram->LinkShaders();

		pointProgram = new ShaderWrapper();
		pointProgram->LoadShader("./shaders/forward_point.vert", VERT_SHADER);
		pointProgram->LoadShader("./shaders/forward_point.frag", FRAG_SHADER);
		pointProgram->LinkShaders();

		spotProgram = new ShaderWrapper();
		spotProgram->LoadShader("./shaders/forward_spot.vert", VERT_SHADER);
		spotProgram->LoadShader("./shaders/forward_spot.frag", FRAG_SHADER);
		spotProgram->LinkShaders();

		/// Material initialisation
		Material crateMat;
		crateMat.ambientColor = glm::vec4(1);
		crateMat.diffuseColor = glm::vec4(1);
		crateMat.specular = glm::vec4(1);
		crateMat.shininessCoefficient = 64.f;
		crateMat.diffuseMap = crateTex;
		crateMat.specularMap = crateSpecularTex;

		Material planeMat;
		planeMat.ambientColor = glm::vec4(1);
		planeMat.diffuseColor = glm::vec4(1);
		planeMat.specular = glm::vec4(1);
		planeMat.shininessCoefficient = 1.f;
		planeMat.diffuseMap	= lightTex;

		/// Mesh initialisation
		testModel = new Model("./models/nanosuit/nanosuit.obj");

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
																				 
			Vertex(glm::vec4(-0.5f, -0.5f,  0.5f,1.f),	glm::vec2(0.f, 0.f), glm::vec4( 0.0f,  0.0f, 1.0f,0.f)),
			Vertex(glm::vec4(0.5f, -0.5f,  0.5f, 1.f),	glm::vec2(1.f, 0.f), glm::vec4(0.0f,  0.0f, 1.0f,0.f)),
			Vertex(glm::vec4(0.5f,  0.5f,  0.5f, 1.f),	glm::vec2(1.f, 1.f), glm::vec4(0.0f,  0.0f, 1.0f,0.f)),
			Vertex(glm::vec4(0.5f,  0.5f,  0.5f, 1.f),	glm::vec2(1.f, 1.f), glm::vec4(0.0f,  0.0f, 1.0f,0.f)),
			Vertex(glm::vec4(-0.5f,  0.5f,  0.5f,1.f),	glm::vec2(0.f, 1.f), glm::vec4( 0.0f,  0.0f, 1.0f,0.f)),
			Vertex(glm::vec4(-0.5f, -0.5f,  0.5f,1.f),	glm::vec2(0.f, 0.f), glm::vec4( 0.0f,  0.0f, 1.0f,0.f)),
																				 
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
																				 
			Vertex(glm::vec4(-0.5f, -0.5f, -0.5f,1.f),  glm::vec2(0.f, 1.f), glm::vec4(	0.0f, -1.0f,  0.0f, 0.f	)),
			Vertex(glm::vec4(0.5f, -0.5f, -0.5f, 1.f),	glm::vec2(1.f, 1.f), glm::vec4(	0.0f, -1.0f,  0.0f,	 0.f)),
			Vertex(glm::vec4(0.5f, -0.5f,  0.5f, 1.f),	glm::vec2(1.f, 0.f), glm::vec4(	0.0f, -1.0f,  0.0f,	 0.f)),
			Vertex(glm::vec4(0.5f, -0.5f,  0.5f, 1.f),	glm::vec2(1.f, 0.f), glm::vec4(	0.0f, -1.0f,  0.0f,	 0.f)),
			Vertex(glm::vec4(-0.5f, -0.5f,  0.5f,1.f),	glm::vec2(0.f, 0.f), glm::vec4(	0.0f, -1.0f,  0.0f, 0.f	)),
			Vertex(glm::vec4(-0.5f, -0.5f, -0.5f,1.f),	glm::vec2(0.f, 1.f), glm::vec4(	0.0f, -1.0f,  0.0f, 0.f	)),

			Vertex(glm::vec4(-0.5f,  0.5f, -0.5f,1.f),	glm::vec2(0.f, 1.f), glm::vec4( 0.0f,  1.0f,  0.0f, 0.f )),
			Vertex(glm::vec4(0.5f,  0.5f, -0.5f, 1.f),	glm::vec2(1.f, 1.f), glm::vec4(	0.0f,  1.0f,  0.0f,	 0.f)),
			Vertex(glm::vec4(0.5f,  0.5f,  0.5f, 1.f),	glm::vec2(1.f, 0.f), glm::vec4(	0.0f,  1.0f,  0.0f,	 0.f)),
			Vertex(glm::vec4(0.5f,  0.5f,  0.5f, 1.f),	glm::vec2(1.f, 0.f), glm::vec4(	0.0f,  1.0f,  0.0f,	 0.f)),
			Vertex(glm::vec4(-0.5f,  0.5f,  0.5f,1.f),	glm::vec2(0.f, 0.f), glm::vec4(	 0.0f,  1.0f,  0.0f, 0.f)),
			Vertex(glm::vec4(-0.5f,  0.5f, -0.5f,1.f),	glm::vec2(0.f, 1.f), glm::vec4(	 0.0f,  1.0f,  0.0f, 0.f))
		};

		for (int i = 0; i < DEFAULT_CUBE_NUM; ++i) {
			sceneMeshes.push_back(new Mesh(cubeVerts, cubeFormat, new Transform(), crateMat));

			sceneMeshes[i]->GetTransform()->SetPosition(glm::vec3(i * 2, i * 2, i * 2));
			sceneMeshes[i]->GetTransform()->SetScale(glm::vec3(1));
		}

		//// Rectangle
		std::vector<Vertex> rectVerts = {
			// Positions				// Tex coords		// Vertex normals
			Vertex(glm::vec4(-0.5f, 0.5f, 0.f, 1.f), glm::vec2(0.f, 1.f), glm::vec4(0.f, 0.f, -1.f, 0.f)),		// Top left
			Vertex(glm::vec4(0.5f, 0.5f, 0.f, 1.f),	 glm::vec2(1.f, 1.f), glm::vec4(0.f, 0.f, -1.f, 0.f)),		// Top right
			Vertex(glm::vec4(-0.5f, -0.5f, 0.f, 1.f),glm::vec2(0.f, 0.f), glm::vec4(0.f, 0.f, -1.f, 0.f)),		// Bottom left
			Vertex(glm::vec4(0.5f, -0.5f, 0.f, 1.f), glm::vec2(1.f, 0.f), glm::vec4(0.f, 0.f, -1.f, 0.f))		// Bottom right
		};

		Transform* rectTransform = new Transform();
		rectTransform->SetScale(glm::vec3(10.f));
		rectTransform->SetRotation(glm::vec3(glm::radians(90.f), 0.f, 0.f));
		rectTransform->Translate(glm::vec3(0, -2.f, 0));

		sceneMeshes.push_back(new Mesh(rectVerts, rectFormat, rectTransform, planeMat));

	}
	else {
		/// Vertex shader
		// Create unique shader ID and bind to GL_VERTEX_SHADER
		unsigned int vertShaderKernel = glCreateShader(GL_VERTEX_SHADER);
		std::string vertShaderString;
		RendererUtility::LoadTextToString("./shaders/base.vert", vertShaderString);

		const char* vertShaderSource = vertShaderString.c_str();			// Convert from string to const char* for compatability with openGL

																			// Set shader source and attempt to compile the GLSL code
		glShaderSource(vertShaderKernel, 1, &vertShaderSource, NULL);		// Second parameter specifies how many strings are being passed in
		glCompileShader(vertShaderKernel);

		/// Fragment shader
		unsigned int fragShaderKernel = glCreateShader(GL_FRAGMENT_SHADER);
		std::string fragShaderString;
		RendererUtility::LoadTextToString("./shaders/base.frag", fragShaderString);

		const char* fragShaderSource = fragShaderString.c_str();

		glShaderSource(fragShaderKernel, 1, &fragShaderSource, NULL);
		glCompileShader(fragShaderKernel);

		/// Shader program
		unsigned int shaderProgKernel = glCreateProgram();

		// Attach successfully compiled shaders and attempt to link
		glAttachShader(shaderProgKernel, vertShaderKernel);
		glAttachShader(shaderProgKernel, fragShaderKernel);

		glLinkProgram(shaderProgKernel);

		// Program initialised successfully, set as current context and clean up the linked individual shaders
		glUseProgram(shaderProgKernel);

		glDeleteShader(vertShaderKernel);
		glDeleteShader(fragShaderKernel);

		/// Vertices
		// Get unique buffer ID for vertices buffer
		unsigned int vertBufferKernel;
		glGenBuffers(1, &vertBufferKernel);

		// Bind buffer to GPU as current context/alias for GL_ARRAY_BUFFER
		glBindBuffer(GL_ARRAY_BUFFER, vertBufferKernel);

		// Define and set verts as buffer data
		static float verts[] = {	// Normalized device coordinates (NDC)
									/// Square
			-0.5f, 0.5f, 0.0f,		// Top left	
			0.5f, 0.5f, 0.0f,		// Top right
			-0.5f, -0.5f, 0.0f,		// Bottom left
			0.5f, -0.5f, 0.0f		// Bottom right
		};
		glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);		/* Buffers do not store data-types, they store bytes and thus need to know the total size of the data
																					Enum gives efficiency hints to openGL, in this case it expects the verts to never change position.*/

																					/// Vertex data interpretation
		unsigned int vertArrayKernel;	// Stores information about vertex attributes and indices
		glGenVertexArrays(1, &vertArrayKernel);

		// NOTE: Vertex attributes and draw orders will only be applied to the currently bound vertex array object
		glBindVertexArray(vertArrayKernel);

		// Position
		glVertexAttribPointer(
			0,							// Vertex attribute ID (specified via layout in the shader)
			3,							// Number of elements in vertex
			GL_FLOAT,					// Type of data in vertex
			GL_FALSE,					// Whether to normalize float values between 0-1 or -1-1 if its signed
			3 * sizeof(float),			// Stride
			NULL);						// Offset

		glEnableVertexAttribArray(0);	// Attributes are disabled by default

										// Vertex draw order
		unsigned int elementBufferKernel;
		glGenBuffers(1, &elementBufferKernel);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferKernel);
		// Define and set indices as buffer data													
		static unsigned int indices[] = {
			0, 1, 2,		// First triangle
			1, 2, 3			// Second triangle
		};

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	}

	/// Draw mode
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);		// Wireframe applied to front and back of triangles

	return 0;
}

void RendererProgram::Shutdown()
{
	aie::Gizmos::destroy();

	delete wallTex;
	delete faceTex;
	delete lightTex;
	delete crateTex;

	delete sphereTransform;
	
	delete mainCamera;

	for (int i = 0; i < sceneMeshes.size(); ++i) {
		delete sceneMeshes[i];
	}
	sceneMeshes.clear();

	for (int i = 0; i < sceneLights.size(); ++i) {
		delete sceneLights[i];
	}
	sceneLights.clear();

	delete ambientProgram;
	delete directionalProgram;
	delete spotProgram;
	delete pointProgram;

	delete testModel;
}

void RendererProgram::FixedUpdate(float a_dt)
{
	static float m_accumulatedTime = 0.f;
	static float m_fixedTimeStep = 0.01f;

	m_accumulatedTime += a_dt;

	// Run update until no more extra time between updates left
	while (m_accumulatedTime >= m_fixedTimeStep) {

		mainCamera->Update(m_fixedTimeStep);

		InputMonitor* input = InputMonitor::GetInstance();

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
				static float scale = 0.01f;

				glm::vec4 orbitVec = glm::vec4(cosf(glfwGetTime()), 0.f, sinf(glfwGetTime()), 0.f) * orbitRadius * scale;

				ptLight->SetPos(ptLight->GetPos() + orbitVec);
			}
		}

		// Turn flash light on and off
		if (input->GetKeyDown(GLFW_KEY_X)) {		// Toggle flashlight
			isFlashLightOn = !isFlashLightOn;
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
			aie::Gizmos::addSphere(ptLight->GetPos(), ptLight->GetIlluminationRadius(), 12, 12, radiusColor);		// Illumination radius
			#endif
		}
	}

#pragma endregion Gizmo creation
}

void RendererProgram::Render()
{
	//aie::Gizmos::draw(projectionMatrix * viewMatrix);
	aie::Gizmos::draw(mainCamera->CalculateProjectionView());

	ShaderWrapper* flashLight = (isFlashLightOn ? spotProgram : nullptr);	// Ignore spot light program pass if flash light isn't on

	if (WRAPPED_OGL_OTHER) {
		for (int i = 0; i < sceneMeshes.size(); ++i) {
			sceneMeshes[i]->Draw(mainCamera, sceneLights, globalAmbient, ambientProgram, directionalProgram, pointProgram, flashLight);
		}

		testModel->Draw(mainCamera, sceneLights, globalAmbient, ambientProgram, directionalProgram, nullptr, nullptr);
	}
	else {
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);	// Renderer shape hint, number of vertices to draw, offset in indice buffer
	}
}