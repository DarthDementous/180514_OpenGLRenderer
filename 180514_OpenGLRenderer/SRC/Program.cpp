#include "Program.h"
#include "Renderer_Utility_Literals.h"

#include <GLFW/glfw3.h>
#include <iostream>

Program::Program()
{
}

Program::~Program()
{
}

int Program::Startup()
{
	return 0;
}

void Program::Shutdown()
{
}

void Program::Update(float a_dt)
{
}

void Program::Render()
{
}

int Program::Run()
{
	if (glfwInit() == false) {		// Failed to initialise
		return EXIT_FAILURE;
	}

	/// Launch window
	GLFWwindow* window = glfwCreateWindow(1280, 720, "OpenGL Renderer", nullptr, nullptr);

	if (!window) {					// Failed to create window
		glfwTerminate();
		return WINDOW_CREATE_FAIL;
	}

	glfwMakeContextCurrent(window);

	/// Initialise openGL functions to correct version
	if (ogl_LoadFunctions() == ogl_LOAD_FAILED) {	// Failed to load version-specific openGL functions
		glfwDestroyWindow(window);
		glfwTerminate();
		return OPENGL_LOAD_FAIL;
	}

	/// Rendering initialisation
	glClearColor(0.25f, 0.25f, 0.25f, 1);
	glEnable(GL_DEPTH_TEST);

	Startup();

	// Keep track of frame times to normalize calculations (use doubles to avoid loss of precision errors)
	double lastFrameTime	= glfwGetTime();
	double currFrameTime	= 0;
	double deltaTime		= 0;

	/// Main loop
	while (glfwWindowShouldClose(window) == false && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) {		// Window has not been closed and escape key has not been pressed
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Wipe back buffers and clear distance to pixels to indicate we're rendering a new frame

		glfwPollEvents();			// Record input

		// Calculate time between frames
		currFrameTime	= glfwGetTime();
		deltaTime		= currFrameTime - lastFrameTime;
		lastFrameTime	= currFrameTime;

		Update((float)deltaTime);

		Render();
		glfwSwapBuffers(window);	// Render graphics in window
	}

	Shutdown();

	glfwDestroyWindow(window);
	glfwTerminate();				// Close down openGL systems and free memory
	return EXIT_SUCCESS;
}
