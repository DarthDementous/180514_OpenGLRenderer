#include "RendererProgram.h"
#include "Transform.h"
#include "RenderCamera.h"
#include "InputMonitor.h"
#include "Renderer_Utility_Funcs.h"
#include "Renderer_Utility_Literals.h"
#include "Material.h"
#include "VertexFormat.h"
#include "Mesh.h"
#include "TextureWrapper.h"
#include "Light\PhongLight_Dir.h"
#include "Light\PhongLight_Point.h"
#include "Light\PhongLight_Spot.h"

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
	dirLight = new PhongLight_Dir(glm::vec4(0.05f), glm::vec4(0.4f), glm::vec4(0.5f), DEFAULT_LIGHT_DIR);
	
	ptLight1 = new PhongLight_Point(glm::vec4(0.05f), glm::vec4(0.8f), glm::vec4(1.f), DEFAULT_LIGHT_POS1, 10.f, DEFAULT_MIN_ILLUMINATION);
	ptLight2 = new PhongLight_Point(glm::vec4(0.05f), glm::vec4(0.8f), glm::vec4(1.f), DEFAULT_LIGHT_POS2, 20.f, DEFAULT_MIN_ILLUMINATION);

	spotLight = new PhongLight_Spot(glm::vec4(0.05f), glm::vec4(1.f), glm::vec4(1.f), glm::vec4(-10.f, 0.f, 0.f, 1.f), glm::vec4(1, 0, 0, 0), 10.f, 14.f);

	/// Texture initialisation
	if (WRAPPED_OGL_TEX) {
		faceTex = new TextureWrapper("./textures/awesomeface.png", 10, true);
		faceTex->EnableFiltering();
		faceTex->EnableMipmapping();
		faceTex->EnableWrapping();

		wallTex = new TextureWrapper("./textures/wall.jpg", 11);
		wallTex->EnableFiltering();
		wallTex->EnableMipmapping();
		wallTex->EnableWrapping();

		lightTex = new TextureWrapper("./textures/light.jpg", 0);
		lightTex->EnableFiltering();
		lightTex->EnableMipmapping();
		lightTex->EnableWrapping();

		crateTex = new TextureWrapper("./textures/container2.png", 12, true);
		crateTex->EnableFiltering();
		crateTex->EnableMipmapping();
		crateTex->EnableWrapping();

		crateSpecularTex = new TextureWrapper("./textures/container2_specular.png", 22, true);
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

	/// Material initialisation
	if (WRAPPED_OGL_OTHER) {
		//// Base material
		Material* baseMat = new Material();

		baseMat->LoadShader("./shaders/base.vert", VERT_SHADER);
		baseMat->LoadShader("./shaders/base_lit_fixed.frag", FRAG_SHADER);

		baseMat->LinkShaders();

		// Modify material parameters
		baseMat->SetVec4("material.ambientColor", glm::vec4(1));
		baseMat->SetVec4("material.diffuseColor", glm::vec4(1));
		baseMat->SetVec4("material.specular", glm::vec4(1));
		baseMat->SetFloat("material.shininessCoefficient", 64.f);

		/// Modify lighting parameters
		// Directional lights
		baseMat->SetVec4("dirLight.ambient", dirLight->GetAmbient());
		baseMat->SetVec4("dirLight.diffuse", dirLight->GetDiffuse());
		baseMat->SetVec4("dirLight.specular", dirLight->GetSpecular());
		baseMat->SetVec4("dirLight.castDir", dirLight->GetCastDir());

		// Point lights
		baseMat->SetVec4("pointLights[0].ambient", ptLight1->GetAmbient());
		baseMat->SetVec4("pointLights[0].diffuse", ptLight1->GetDiffuse());
		baseMat->SetVec4("pointLights[0].specular", ptLight1->GetSpecular());
		baseMat->SetVec4("pointLights[0].position", ptLight1->GetPos());
		baseMat->SetFloat("pointLights[0].illuminationRadius", ptLight1->GetIlluminationRadius());
		baseMat->SetFloat("pointLights[0].minIllumination", ptLight1->GetMinIllumination());

		baseMat->SetVec4("pointLights[1].ambient", ptLight2->GetAmbient());
		baseMat->SetVec4("pointLights[1].diffuse", ptLight2->GetDiffuse());
		baseMat->SetVec4("pointLights[1].specular", ptLight2->GetSpecular());
		baseMat->SetVec4("pointLights[1].position", ptLight2->GetPos());
		baseMat->SetFloat("pointLights[1].illuminationRadius", ptLight2->GetIlluminationRadius());
		baseMat->SetFloat("pointLights[1].minIllumination", ptLight2->GetMinIllumination());

		// Spot lights
		baseMat->SetVec4("spotLight.ambient", spotLight->GetAmbient());
		baseMat->SetVec4("spotLight.diffuse", spotLight->GetDiffuse());
		baseMat->SetVec4("spotLight.specular", spotLight->GetSpecular());
		baseMat->SetVec4("spotLight.position", spotLight->GetPos());
		baseMat->SetVec4("spotLight.spotDir", spotLight->GetSpotDir());
		baseMat->SetFloat("spotLight.spotInnerCosine", spotLight->GetSpotInnerCosine());
		baseMat->SetFloat("spotLight.spotOuterCosine", spotLight->GetSpotOuterCosine());

		// Texture assigning
		baseMat->SetTexture("diffuseMap", crateTex);
		baseMat->SetTexture("specularMap", crateSpecularTex);

		//// Light material
		Material* lightMat = new Material();

		lightMat->LoadShader("./shaders/base.vert", VERT_SHADER);
		lightMat->LoadShader("./shaders/base.frag", FRAG_SHADER);
		lightMat->LinkShaders();

		lightMat->SetTexture("textureSample0", lightTex);

		//// Plane material
		Material* rectMat = new Material();

		rectMat->LoadShader("./shaders/base.vert", VERT_SHADER);
		rectMat->LoadShader("./shaders/base_lit_color.frag", FRAG_SHADER);
		rectMat->LinkShaders();

		rectMat->SetFloat("ambientCoefficient", 0.1f);
		rectMat->SetVec4("lightColor", glm::vec4(1));
		rectMat->SetVec4("lightPos", DEFAULT_LIGHT_POS1);

		rectMat->SetFloat("specularCoefficient", 0.5f);
		rectMat->SetFloat("shininessCoefficient", 32);

		/// Mesh initialisation
		// Vertex formats
		VertexFormat* rectFormat = new VertexFormat(std::vector<unsigned int>{
			0, 1, 2,		// First triangle
			1, 2, 3			// Second triangle
		});

		VertexFormat* rhombusFormat = new VertexFormat(std::vector<unsigned int>{
			0, 1, 3,		// First triangle
			1, 2, 3			// Second triangle
		});
		
		VertexFormat* cubeFormat = new VertexFormat(std::vector<unsigned int>{0});

		// Meshes
		rectMesh = new Mesh(std::vector<float>{
			/// Square
			// Positions				// Colors					// Tex coords			// Vertex normals
			-0.5f, 0.5f, 0.0f, 1.f,		1.0f, 0.5f, 0.31f, 1.f,		0.0f, 1.0f,				0.f, 0.f, -1.f, 0.f,			// Top left	
			0.5f, 0.5f, 0.0f, 1.f,		1.0f, 0.5f, 0.31f, 1.f,		1.f, 1.f,				0.f, 0.f, -1.f, 0.f,			// Top right
			-0.5f, -0.5f, 0.0f,	1.f,	1.0f, 0.5f, 0.31f, 1.f,		0.f, 0.f,				0.f, 0.f, -1.f, 0.f,			// Bottom left
			0.5f, -0.5f, 0.0f, 1.f,		1.0f, 0.5f, 0.31f, 1.f,		1.f, 0.f,				0.f, 0.f, -1.f, 0.f				// Bottom right
		}, COLOR_TEXTURE_NORMAL, rectMat, rectFormat, new Transform());
		rectMesh->GetTransform()->SetPosition(glm::vec3(0, 0, 0));
		rectMesh->GetTransform()->SetScale(glm::vec3(10, 10, 10));
		rectMesh->GetTransform()->SetRotation(glm::vec3(glm::radians(90.f), 0, 0));

		rhombusMesh = new Mesh(std::vector<float>{
			-0.5f, -0.9f, 1.f, 1.f,		0.5f, 0.f, 0.f, 1.f,	0.0f, 1.0f,
			-0.25f, -0.65f, 1.f, 1.f,	0.5f, 0.f, 0.f, 1.f,	1.f, 1.f,
			0.25f, -0.65f, 1.f, 1.f,	0.5f, 0.f, 0.f, 1.f,	0.f, 0.f,
			0.5f, -0.9f, 1.f, 1.f,		0.5f, 0.f, 0.f, 1.f,	1.f, 0.f
		}, COLOR_TEXTURE, baseMat, rhombusFormat, new Transform());
		rhombusMesh->GetTransform()->SetPosition(glm::vec3(-10, 0, 0));

		std::vector<float> cubeVerts = {
			-0.5f, -0.5f, -0.5f,1.f,		0.f, 0.f,			 0.0f,  0.0f, -1.0f, 0.f,
			0.5f, -0.5f, -0.5f,	1.f,		1.f, 0.f,			0.0f,  0.0f, -1.0f,	 0.f,
			0.5f,  0.5f, -0.5f,	1.f,		1.f, 1.f,			0.0f,  0.0f, -1.0f,	 0.f,
			0.5f,  0.5f, -0.5f,	1.f,		1.f, 1.f,			0.0f,  0.0f, -1.0f,	 0.f,
			-0.5f,  0.5f, -0.5f,1.f,		0.f, 1.f,			 0.0f,  0.0f, -1.0f, 0.f,
			-0.5f, -0.5f, -0.5f,1.f,		0.f, 0.f,			 0.0f,  0.0f, -1.0f, 0.f,
																				 
			-0.5f, -0.5f,  0.5f,1.f,		0.f, 0.f,			 0.0f,  0.0f, 1.0f,	 0.f,
			0.5f, -0.5f,  0.5f,	1.f,		1.f, 0.f,			0.0f,  0.0f, 1.0f,	 0.f,
			0.5f,  0.5f,  0.5f,	1.f,		1.f, 1.f,			0.0f,  0.0f, 1.0f,	 0.f,
			0.5f,  0.5f,  0.5f,	1.f,		1.f, 1.f,			0.0f,  0.0f, 1.0f,	 0.f,
			-0.5f,  0.5f,  0.5f,1.f,		0.f, 1.f,			 0.0f,  0.0f, 1.0f,	 0.f,
			-0.5f, -0.5f,  0.5f,1.f,		0.f, 0.f,			 0.0f,  0.0f, 1.0f,	 0.f,
																				 
			-0.5f,  0.5f,  0.5f,1.f,		1.f, 0.f,			-1.0f,  0.0f,  0.0f, 0.f,
			-0.5f,  0.5f, -0.5f,1.f,		1.f, 1.f,			-1.0f,  0.0f,  0.0f, 0.f,
			-0.5f, -0.5f, -0.5f,1.f,		0.f, 1.f,			-1.0f,  0.0f,  0.0f, 0.f,
			-0.5f, -0.5f, -0.5f,1.f,		0.f, 1.f,			-1.0f,  0.0f,  0.0f, 0.f,
			-0.5f, -0.5f,  0.5f,1.f,		0.f, 0.f,			-1.0f,  0.0f,  0.0f, 0.f,
			-0.5f,  0.5f,  0.5f,1.f,		1.f, 0.f,			-1.0f,  0.0f,  0.0f, 0.f,
																				 
			0.5f,  0.5f,  0.5f,	1.f,		1.f, 0.f,			1.0f,  0.0f,  0.0f,	 0.f,
			0.5f,  0.5f, -0.5f,	1.f,		1.f, 1.f,			1.0f,  0.0f,  0.0f,	 0.f,
			0.5f, -0.5f, -0.5f,	1.f,		0.f, 1.f,			1.0f,  0.0f,  0.0f,	 0.f,
			0.5f, -0.5f, -0.5f,	1.f,		0.f, 1.f,			1.0f,  0.0f,  0.0f,	 0.f,
			0.5f, -0.5f,  0.5f,	1.f,		0.f, 0.f,			1.0f,  0.0f,  0.0f,	 0.f,
			0.5f,  0.5f,  0.5f,	1.f,		1.f, 0.f,			1.0f,  0.0f,  0.0f,	 0.f,
																				 
			-0.5f, -0.5f, -0.5f,1.f,		0.f, 1.f,			 0.0f, -1.0f,  0.0f, 0.f,
			0.5f, -0.5f, -0.5f,	1.f,		1.f, 1.f,			0.0f, -1.0f,  0.0f,	 0.f,
			0.5f, -0.5f,  0.5f,	1.f,		1.f, 0.f,			0.0f, -1.0f,  0.0f,	 0.f,
			0.5f, -0.5f,  0.5f,	1.f,		1.f, 0.f,			0.0f, -1.0f,  0.0f,	 0.f,
			-0.5f, -0.5f,  0.5f,1.f,		0.f, 0.f,			 0.0f, -1.0f,  0.0f, 0.f,
			-0.5f, -0.5f, -0.5f,1.f,		0.f, 1.f,			 0.0f, -1.0f,  0.0f, 0.f,
																				 
			-0.5f,  0.5f, -0.5f,1.f,		0.f, 1.f,			 0.0f,  1.0f,  0.0f, 0.f,
			0.5f,  0.5f, -0.5f,	1.f,		1.f, 1.f,			0.0f,  1.0f,  0.0f,	 0.f,
			0.5f,  0.5f,  0.5f,	1.f,		1.f, 0.f,			0.0f,  1.0f,  0.0f,	 0.f,
			0.5f,  0.5f,  0.5f,	1.f,		1.f, 0.f,			0.0f,  1.0f,  0.0f,	 0.f,
			-0.5f,  0.5f,  0.5f,1.f,		0.f, 0.f,			 0.0f,  1.0f,  0.0f, 0.f,
			-0.5f,  0.5f, -0.5f,1.f,		0.f, 1.f,			 0.0f,  1.0f,  0.0f, 0.f
		};

		for (int i = 0; i < DEFAULT_CUBE_NUM; ++i) {
			cubeMeshes.push_back(new Mesh(cubeVerts, TEXTURE_NORMAL,
				baseMat, cubeFormat, new Transform()));

			cubeMeshes[i]->GetTransform()->SetPosition(glm::vec3(i * 2, i * 2, i * 2));
			cubeMeshes[i]->GetTransform()->SetScale(glm::vec3(1));
		}

		lightRepMesh = new Mesh(cubeVerts, TEXTURE_NORMAL, lightMat, cubeFormat, new Transform());
		lightRepMesh->GetTransform()->SetPosition(DEFAULT_LIGHT_POS1);
		lightRepMesh->GetTransform()->SetScale(glm::vec3(0.2));

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
	delete rectMesh;
	delete rhombusMesh;

	delete wallTex;
	delete faceTex;
	delete lightTex;
	delete crateTex;

	delete dirLight;
	delete ptLight1;
	delete ptLight2;
	delete spotLight;

	delete sphereTransform;
	
	delete mainCamera;
}

void RendererProgram::Update(float a_dt)
{
	mainCamera->Update(a_dt);

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
	aie::Gizmos::addSphere(ptLight1->GetPos(), 0.2f, 12, 12, ptLight1->GetAmbient());
	aie::Gizmos::addSphere(ptLight2->GetPos(), 0.2f, 12, 12, ptLight2->GetAmbient());

	// Spot lights
	//aie::Gizmos::addSphere(spotLight->GetPos(), 0.2f, 12, 12, spotLight->GetAmbient());

#ifdef DEBUG	// Debug draw point light radius
	glm::vec4 light1Color = glm::vec4(ptLight1->GetDiffuse().x, ptLight1->GetDiffuse().y, ptLight1->GetDiffuse().z, 0.25f);
	aie::Gizmos::addSphere(ptLight1->GetPos(), ptLight1->GetIlluminationRadius(), 12, 12, light1Color);		// Illumination radius

	glm::vec4 light2Color = glm::vec4(ptLight2->GetDiffuse().x, ptLight2->GetDiffuse().y, ptLight2->GetDiffuse().z, 0.25f);
	aie::Gizmos::addSphere(ptLight2->GetPos(), ptLight2->GetIlluminationRadius(), 12, 12, light2Color);		// Illumination radius
#endif
#pragma endregion Gizmo creation

	/// Lighting update
	// Update spot light position and direction based off camera
	spotLight->SetPos(glm::vec4(mainCamera->GetTransform()->GetPosition(), 1));
	spotLight->SetSpotDir(glm::vec4(mainCamera->GetTransform()->Forward(), 0));

	if (ORBIT_LIGHT) {
		// Update orbit position of light
		static float orbitRadius = 10.f;
		static float orbitHeight = 2.f;

		glm::vec3 orbitPos = glm::vec3(cosf(glfwGetTime()) * orbitRadius, orbitHeight, sinf(glfwGetTime()) * orbitRadius); 	// Get point on orbit circumference based off current angle

		//lightRepMesh->GetTransform()->SetPosition(orbitPos);
		ptLight2->SetPos(glm::vec4(orbitPos, 1));

		/// Input
		InputMonitor* input = InputMonitor::GetInstance();
		float movementSpeed = 10.f;

		if (input->GetKeyDown(GLFW_KEY_SPACE)) {		// Move light up
			orbitHeight += movementSpeed * a_dt;
		}
		if (input->GetKeyDown(GLFW_KEY_LEFT_SHIFT)) {	// Move light down
			orbitHeight -= movementSpeed * a_dt;
		}
		if (input->GetKeyDown(GLFW_KEY_DOWN)) {			// Move light out
			orbitRadius += movementSpeed * a_dt;
		}
		if (input->GetKeyDown(GLFW_KEY_UP)) {			// Move light in
			orbitRadius -= movementSpeed * a_dt;
		}
	}

	// Update light information on materials
	glm::vec4 newLightPos = ptLight2->GetPos();//glm::vec4(lightRepMesh->GetTransform()->GetPosition(), 1);//(mainCamera->GetTransform()->GetPosition(), 1);

	for (int i = 0; i < DEFAULT_CUBE_NUM; ++i) {
		//cubeMeshes[i]->GetTransform()->SetRotation(glm::vec3(float(glfwGetTime()), float(glfwGetTime()), float(glfwGetTime())));
		cubeMeshes[i]->GetMaterial()->SetVec4("pointLights[1].position", newLightPos);

		cubeMeshes[i]->GetMaterial()->SetVec4("spotLight.position", spotLight->GetPos());
		cubeMeshes[i]->GetMaterial()->SetVec4("spotLight.spotDir", spotLight->GetSpotDir());
	}

	//rectMesh->GetMaterial()->SetVec4("lightPos", newLightPos);
}

void RendererProgram::Render()
{
	//aie::Gizmos::draw(projectionMatrix * viewMatrix);
	aie::Gizmos::draw(mainCamera->CalculateProjectionView());

	if (WRAPPED_OGL_OTHER) {
		//triMesh->GetMaterial()->SetVec4("overrideColor", glm::vec4(0, 1.f, 1.f, 1.f));		// If meshes share the same material, then modifying variables in one material will apply to all
		static float offset = 0.5f;
		//rectMesh->GetMaterial()->SetFloat("yOffset", offset);

		//rectMesh->Draw(mainCamera);
		//rhombusMesh->Draw(mainCamera);
		
		//lightRepMesh->Draw(mainCamera);

		for (int i = 0; i < DEFAULT_CUBE_NUM; ++i) {
			cubeMeshes[i]->Draw(mainCamera);
		}

		offset -= 0.001f;
	}
	else {
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);	// Renderer shape hint, number of vertices to draw, offset in indice buffer
	}
}
