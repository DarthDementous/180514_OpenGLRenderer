#pragma once

#define WRAPPED_OGL_TEX true
#define WRAPPED_OGL_OTHER true
#define PRESET_FORMAT_DRAW false

#define DEFAULT_LIGHT_POS glm::vec4(1.2f, 1.f, -5.f, 1)
#define DEFAULT_CUBE_NUM 10
enum eProgramError {
	GLFW_INITIALISE_FAIL = 10,
	WINDOW_CREATE_FAIL,
	OPENGL_LOAD_FAIL,
};

enum eShaderType {
	FRAG_SHADER,
	VERT_SHADER
};