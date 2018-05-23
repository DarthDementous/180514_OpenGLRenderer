/*
Written by Sebastian Toy.
*/

#include <gl_core_4_4.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext.hpp>
#include <AIE/Gizmos.h>

enum eRendererError {
	WINDOW_CREATE_FAIL = 10,
	OPENGL_LOAD_FAIL
};

int main2() {

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

	aie::Gizmos::create(1000, 1000, 1000, 1000);

	glm::mat4 viewMatrix			= glm::lookAt(glm::vec3(10, 10, 10), glm::vec3(0), glm::vec3(0, 1, 0));		// Create view matrix looking at an arbitrary point
	glm::mat4 projectionMatrix		= glm::perspective(glm::pi<float>() * 0.25f, 16 / 9.f, 0.1f, 1000.f);		// Define how objects should be drawn

	/// Main loop
	while (glfwWindowShouldClose(window) == false && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) {		// Window has not been closed and escape key has not been pressed
		/// Rendering tests
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Wipe back buffers and clear distance to pixels to indicate we're rendering a new frame
		aie::Gizmos::clear();									// Refresh gizmos for new frame

		aie::Gizmos::addTransform(glm::mat4(1));				// Visually represent identity matrix with unit vectors

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

		aie::Gizmos::draw(projectionMatrix * viewMatrix);

		glfwPollEvents();			// Record input
		glfwSwapBuffers(window);	// Render graphics in window
	}

	aie::Gizmos::destroy();
	glfwDestroyWindow(window);
	glfwTerminate();				// Close down openGL systems and free memory
	return EXIT_SUCCESS;
}