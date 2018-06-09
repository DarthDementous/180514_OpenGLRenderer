#pragma once

#define WRAPPED_OGL_TEX true
#define WRAPPED_OGL_OTHER true
#define PRESET_FORMAT_DRAW false

#define ENABLE_POINT_LIGHTS false
#define ENABLE_SPOT_LIGHTS false
#define ENABLE_DIR_LIGHTS true

#define ENABLE_POST_PROCESSING true
#define ENABLE_GAMMA_CORRECT true
#define ENABLE_SHARPEN false
#define ENABLE_BLUR false
#define ENABLE_EDGE_DETECT false

#define BLEND_POST_PROCESSING true
#define BLEND_RENDERING true

#define DEFAULT_CLEAR_COLOR 0.2f, 0.2f, 0.25f, 1
#define DEFAULT_GLOBAL_AMBIENT glm::vec4(0.01f, 0.01f, 0.01f, 1)
#define DRAW_WIREFRAME false
#define DRAW_GIZMOS true
#define DRAW_NORMALS false
#define USE_GIZMO_CAMERA false

#define ERROR_CHECK_UNIFORM_FIND false
#define ERROR_CHECK_OPENGL true

#define ORBIT_LIGHT true
#define DEFAULT_LIGHT_POS1 glm::vec4(10.2f, 13.f, 10.f, 1.f)
#define DEFAULT_LIGHT_POS2 glm::vec4(1.5f, 3.f, -4.f, 1.f)
#define DEFAULT_LIGHT_DIR glm::vec4(0.f, -1.f, 1.f, 0.f)
#define DEFAULT_CUBE_NUM 0
#define DEFAULT_MIN_ILLUMINATION 0.001f
#define SKY_COLOR glm::vec4(64.f / 255, 156.f / 255, 255.f / 255, 1.f)

namespace SPRON {
	enum eProgramError {
		GLFW_INITIALISE_FAIL = 10,
		WINDOW_CREATE_FAIL,
		OPENGL_LOAD_FAIL,
	};

	enum eShaderType {
		FRAG_SHADER,
		VERT_SHADER,
		GEOMETRY_SHADER
	};
}