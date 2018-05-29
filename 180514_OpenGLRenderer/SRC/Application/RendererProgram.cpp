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
	aie::Gizmos::create(1000, 1000, 1000, 1000);

	/// Variable initialisation
	viewMatrix = glm::lookAt(glm::vec3(-10, 10, -10), glm::vec3(0), glm::vec3(0, 1, 0));			// Create view matrix looking at at an arbitrary point
	projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, 16 / 9.f, 0.1f, 1000.f);			// Define how objects should be drawn

	sphereTransform = new Transform(glm::vec3(0, 2, 8), glm::vec3(3, 3, 3), glm::vec3(glm::radians(90.f), 0, 0));

	mainCamera = new RenderCamera();
	mainCamera->SetProjection(glm::pi<float>() * 0.25f, 16 / 9.f, 0.1f, 1000.f);
	mainCamera->GetTransform()->SetPosition(glm::vec3(0, 5, -5));
	mainCamera->GetTransform()->SetRotation(glm::vec3(glm::radians(20.f), 0, 0));

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
		Material* baseMat = new Material();

		baseMat->LoadShader("./shaders/base.vert", VERT_SHADER);
		baseMat->LoadShader("./shaders/base.frag", FRAG_SHADER);

		baseMat->LinkShaders();

		// Modify material parameters
		baseMat->SetFloat("xOffset", 0.f);
		baseMat->SetFloat("yOffset", 0.f);

		// Texture assigning
		baseMat->SetTexture("textureSample0", faceTex);
		baseMat->SetTexture("textureSample1", wallTex);

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

		// Meshes
		rectMesh = new Mesh(std::vector<float>{
			/// Square
			// Positions				// Colors				// Tex coords
			-0.5f, 0.5f, 0.0f, 1.f,		1.f, 0.f, 0.f, 1.f,		0.0f, 1.0f,			// Top left	
			0.5f, 0.5f, 0.0f, 1.f,		0.f, 1.f, 0.f, 1.f,		1.f, 1.f,			// Top right
			-0.5f, -0.5f, 0.0f,	1.f,	0.f, 0.f, 1.f, 1.f,		0.f, 0.f,			// Bottom left
			0.5f, -0.5f, 0.0f, 1.f,		1.f, 1.f, 0.f, 1.f,		1.f, 0.f			// Bottom right
		}, baseMat, rectFormat);

		rhombusMesh = new Mesh(std::vector<float>{
			-0.5f, -0.9f, 0.5f, 1.f,	0.5f, 0.f, 0.f, 1.f,	0.0f, 1.0f,
			-0.25f, -0.65f, 0.f, 1.f,	0.5f, 0.f, 0.f, 1.f,	1.f, 1.f,
			0.25f, -0.65f, 0.f, 1.f,	0.5f, 0.f, 0.f, 1.f,	0.f, 0.f,
			0.5f, -0.9f, 0.f, 1.f,		0.5f, 0.f, 0.f, 1.f,	1.f, 0.f
		}, baseMat, rhombusFormat);
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

	// Sphere
	aie::Gizmos::addSphere(glm::vec3(0), 2, 12, 12, glm::vec4(1, 0, 0, 1), &sphereTransform->GetMatrix());
#pragma endregion Gizmo creation

	/// Input
	InputMonitor* input = InputMonitor::GetInstance();
	static float colorMix = 0.f;
	static float colorChangeSpeed = 1.f;

	if (input->GetKeyDown(GLFW_KEY_UP)) {	// Increase interpolation value
		colorMix += colorChangeSpeed * a_dt;
	}
	if (input->GetKeyDown(GLFW_KEY_DOWN)) {	// Decrease interpolation value
		colorMix -= colorChangeSpeed * a_dt;
	}

	rectMesh->GetMaterial()->SetFloat("colorMix", colorMix);
}

void RendererProgram::Render()
{
	//aie::Gizmos::draw(projectionMatrix * viewMatrix);
	aie::Gizmos::draw(mainCamera->CalculateProjectionView());

	if (WRAPPED_OGL_OTHER) {
		//triMesh->GetMaterial()->SetVec4("overrideColor", glm::vec4(0, 1.f, 1.f, 1.f));		// If meshes share the same material, then modifying variables in one material will apply to all
		static float offset = 0.5f;
		rectMesh->GetMaterial()->SetFloat("yOffset", offset);

		rectMesh->Draw();
		rhombusMesh->Draw();

		offset -= 0.001f;
	}
	else {
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);	// Renderer shape hint, number of vertices to draw, offset in indice buffer
	}
}
