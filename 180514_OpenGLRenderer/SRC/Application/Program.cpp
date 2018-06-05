#include "Program.h"
#include "Renderer_Utility_Literals.h"
#include "Renderer_Utility_Funcs.h"

#include <GLFW/glfw3.h>
#include <gl_core_4_4.h>
#include <iostream>
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

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

GLFWwindow* Program::InitialiseWindow(const char* a_windowName, int a_width, int a_height)
{
	if (glfwInit() == false) {		// Failed to initialise
		return nullptr;
	}

#ifdef DEBUG
	// Send debug output context to window for easier debugging
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

	// Set sample rate to 16 (maximum of 16 neighboring texels)
	glfwWindowHint(GLFW_SAMPLES, 16);

	// Create window
	GLFWwindow* window = glfwCreateWindow(a_width, a_height, a_windowName, nullptr, nullptr);

	return window;
}

void Program::DestroyContextWindow()
{
	// Clean up glfw window and loaded glfw data
	glfwDestroyWindow(glfwGetCurrentContext());
	glfwTerminate();
}

int Program::Run(const char* a_windowName, int a_width, int a_height)
{
	/// Launch window
	GLFWwindow* window = InitialiseWindow(a_windowName, 1280, 720);

	if (!window) {					// Failed to create window
		glfwTerminate();
		return WINDOW_CREATE_FAIL;
	}

	glfwMakeContextCurrent(window);

	/// Initialise IMGUI
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui_ImplGlfwGL3_Init(window, true);

	// Style
	ImGui::StyleColorsDark();

	/// Initialise openGL functions to correct version
	if (ogl_LoadFunctions() == ogl_LOAD_FAILED) {	// Failed to load version-specific openGL functions
		DestroyContextWindow();
		return OPENGL_LOAD_FAIL;
	}

#ifdef DEBUG
	// Enable openGL debug context if allowed
	int flag; glGetIntegerv(GL_CONTEXT_FLAGS, &flag);
	if (flag & GL_CONTEXT_FLAG_DEBUG_BIT) {		// Debug context is supported

		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);	// Run callback function the instant an error occurs
		glDebugMessageCallback(RendererUtility::glDebugOutputCallback, nullptr);	// Set callback function
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
#endif

	/// Rendering initialisation
	glClearColor(0.2f, 0.2f, 0.25f, 1);
	glEnable(GL_DEPTH_TEST);	// Activate the z-buffer to make sure the closest pixels draw in overlap scenarios

	// Enable anti-aliasing at the sample rate set by the window hint
	glEnable(GL_MULTISAMPLE);

	Startup();

	// Keep track of frame times to normalize calculations (use doubles to avoid loss of precision errors)
	double lastFrameTime	= glfwGetTime();
	double currFrameTime	= 0;
	double deltaTime		= 0;

	/// Main loop
	while (glfwWindowShouldClose(window) == false && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) {		// Window has not been closed and escape key has not been pressed
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Wipe back buffers and clear z-buffer to indicate we're rendering a new frame

		glfwPollEvents();				// Record input for this frame
		ImGui_ImplGlfwGL3_NewFrame();	// Clear IMGUI for new frame

		// Calculate time between frames
		currFrameTime	= glfwGetTime();
		deltaTime		= currFrameTime - lastFrameTime;
		lastFrameTime	= currFrameTime;

		Update((float)deltaTime);

		Render();

		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);	// Back buffer has received draw information from Render, swap with front buffer to display new graphics for this frame
	}

	// Clean up IMGUI
	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();

	Shutdown();

	DestroyContextWindow();
	return EXIT_SUCCESS;
}
