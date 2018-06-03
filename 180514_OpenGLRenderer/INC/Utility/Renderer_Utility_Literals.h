#pragma once

#define WRAPPED_OGL_TEX true
#define WRAPPED_OGL_OTHER true
#define PRESET_FORMAT_DRAW false

#define ORBIT_LIGHT true
#define DEFAULT_LIGHT_POS1 glm::vec4(10.2f, 13.f, 10.f, 1.f)
#define DEFAULT_LIGHT_POS2 glm::vec4(1.5f, 3.f, -4.f, 1.f)
#define DEFAULT_LIGHT_DIR glm::vec4(0, -1, 0, 0)//glm::vec4(-0.2f, -1.f, -0.3f, 0.f)
#define DEFAULT_CUBE_NUM 10
#define DEFAULT_MIN_ILLUMINATION 0.001f


enum eProgramError {
	GLFW_INITIALISE_FAIL = 10,
	WINDOW_CREATE_FAIL,
	OPENGL_LOAD_FAIL,
};

enum eShaderType {
	FRAG_SHADER,
	VERT_SHADER
};