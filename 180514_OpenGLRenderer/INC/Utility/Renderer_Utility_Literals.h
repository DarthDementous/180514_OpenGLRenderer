#pragma once

#define WRAPPED_OGL_TEX true
#define WRAPPED_OGL_OTHER true

enum eProgramError {
	GLFW_INITIALISE_FAIL = 10,
	WINDOW_CREATE_FAIL,
	OPENGL_LOAD_FAIL,
};

enum eShaderType {
	FRAG_SHADER,
	VERT_SHADER
};