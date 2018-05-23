#include "RendererProgram.h"
#include "Transform.h"
#include "RenderCamera.h"
#include "InputMonitor.h"

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
	viewMatrix			= glm::lookAt(glm::vec3(-10, 10, -10), glm::vec3(0), glm::vec3(0, 1, 0));			// Create view matrix looking at at an arbitrary point
	projectionMatrix	= glm::perspective(glm::pi<float>() * 0.25f, 16 / 9.f, 0.1f, 1000.f);			// Define how objects should be drawn
	
	sphereTransform = new Transform(glm::vec3(0, 2, 8));

	mainCamera = new RenderCamera();
	mainCamera->SetProjection(glm::pi<float>() * 0.25f, 16 / 9.f, 0.1f, 1000.f);
	mainCamera->GetTransform()->SetPosition(glm::vec3(0, 5, -5));
	mainCamera->GetTransform()->SetRotation(glm::vec3(glm::radians(20.f), 0, 0));

	return 0;
}

void RendererProgram::Shutdown()
{
	aie::Gizmos::destroy();
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

	/// Vertices
	// Get unique buffer ID for vertices buffer
	unsigned int vertBufferKernel;
	glGenBuffers(1, &vertBufferKernel);

	// Bind buffer to GPU as current context/alias for GL_ARRAY_BUFFER
	glBindBuffer(GL_ARRAY_BUFFER, vertBufferKernel);

	// Define and set verts as buffer data
	static float singleTriVerts[] = {				// Normalized device coordinates (NDC)
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.0f, 0.5f, 0.0f
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(singleTriVerts), singleTriVerts, GL_STATIC_DRAW);		/* Buffers do not store data-types, they store bytes and thus need to know the total size of the data
																								Enum gives efficiency hints to openGL, in this case it expects the verts to never change position.*/
	/// Vertex shader
	// Create unique shader ID and bind to GL_VERTEX_SHADER
	unsigned int vertShaderKernel = glCreateShader(GL_VERTEX_SHADER);	// TODO: Check whether this breaks by being on the same line

	// Load shader file into a single string
	std::ifstream textFile("./shaders/base.vert");
	std::string vertShaderString;
	
	textFile.seekg(0, std::ios::end);					// Go to end of text file
	vertShaderString.reserve(textFile.tellg());			// Allocate memory for string by getting size in bytes of text file by basing it off the location at the end of text file 
	textFile.seekg(0, std::ios::beg);					// Go to beginning of text file

	vertShaderString.assign(std::istreambuf_iterator<char>(textFile), std::istreambuf_iterator<char>());	// Stream text file from beginning to end into a string
	const char* vertShaderSource = vertShaderString.c_str();												// Convert from string to const char* for compatability with openGL

	// Set shader source and attempt to compile the GLSL code
	glShaderSource(vertShaderKernel, 1, &vertShaderSource, NULL);	// Second parameter specifies how many strings are being passed in
	glCompileShader(vertShaderKernel);

	int successID;
	char infoLog[512];
	glGetShaderiv(vertShaderKernel, GL_COMPILE_STATUS, &successID);

	if (!successID) {		// Compilation was not successful
		glGetShaderInfoLog(vertShaderKernel, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		assert(false);
	}
	else {					// Compilation was successful
		std::cout << "SUCCESS::SHADER::VERTEX::COMPILATION_SUCCESS\n" << std::endl;
	}


}

void RendererProgram::Render()
{
	//aie::Gizmos::draw(projectionMatrix * viewMatrix);
	aie::Gizmos::draw(mainCamera->CalculateProjectionView());
}
