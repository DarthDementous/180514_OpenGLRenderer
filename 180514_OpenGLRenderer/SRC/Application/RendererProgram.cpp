#include "RendererProgram.h"
#include "Transform.h"
#include "RenderCamera.h"
#include "InputMonitor.h"
#include "Renderer_Utility_Funcs.h"
#include "Renderer_Utility_Literals.h"
#include "Material.h"
#include "VertexFormat.h"
#include "Mesh.h"

#include <AIE/Gizmos.h>
#include <glm/vec4.hpp>
#include <glm/ext.hpp>
#include <iostream>
#include <GLFW\glfw3.h>
#include <fstream>
#include <string>
#include <streambuf>

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

	sphereTransform = new Transform(glm::vec3(0, 2, 8));

	mainCamera = new RenderCamera();
	mainCamera->SetProjection(glm::pi<float>() * 0.25f, 16 / 9.f, 0.1f, 1000.f);
	mainCamera->GetTransform()->SetPosition(glm::vec3(0, 5, -5));
	mainCamera->GetTransform()->SetRotation(glm::vec3(glm::radians(20.f), 0, 0));

#pragma region OpenGL
#if true // Wrapped OpenGL
	/// Material initialisation
	Material* baseMat = new Material();

	baseMat->LoadShader("./shaders/base.vert", VERT_SHADER);
	baseMat->LoadShader("./shaders/base.frag", FRAG_SHADER);
		  
	baseMat->LinkShaders();

	/// Mesh initialisation
	// Vertex formats
	VertexFormat* triFormat = new VertexFormat(std::vector<unsigned int>{
		0, 1, 2,		// First triangle
		1, 2, 3			// Second triangle
	});

	VertexFormat* rhombusFormat = new VertexFormat(std::vector<unsigned int>{
		0, 1, 2, 3, 0
	});

	// Meshes
	triMesh = new Mesh( std::vector<float>{
		/// Square
		-0.5f, 0.5f, 0.0f,		// Top left	
		0.5f, 0.5f, 0.0f,		// Top right
		-0.5f, -0.5f, 0.0f,		// Bottom left
		0.5f, -0.5f, 0.0f		// Bottom right
	}, baseMat, triFormat);

	rhombusMesh = new Mesh(std::vector<float>{
		-0.5f, -0.9f, 0.f,
		0.25f, -0.75f, 0.f,
		0.f, -0.75f, 0.f,
		0.5f, -0.9f, 0.f
	}, baseMat, rhombusFormat);

#else	  // Un-wrapped 'pure' OpenGL
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
#endif
#pragma endregion

	/// Draw mode
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);		// Wireframe applied to front and back of triangles

	return 0;
}

void RendererProgram::Shutdown()
{
	aie::Gizmos::destroy();
	delete triMesh;
	delete rhombusMesh;
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

}

void RendererProgram::Render()
{
	//aie::Gizmos::draw(projectionMatrix * viewMatrix);
	//aie::Gizmos::draw(mainCamera->CalculateProjectionView());

#if true	// Wrapped draw method
	triMesh->Draw();
	rhombusMesh->Draw();

#else		// Un-wrapped draw method
	// Draw OpenGL triangle
	#if false	/// Array-order method
		glDrawArrays(GL_TRIANGLES, 0, 6);		// Renderer shape hint, starting index of vertex array, number of elements in vertex array to process
	#else		/// Element-order method
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);	// Renderer shape hint, number of vertices to draw, offset in indice buffer
	#endif
#endif
}
