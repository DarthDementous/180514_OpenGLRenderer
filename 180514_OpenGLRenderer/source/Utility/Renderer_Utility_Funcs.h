#pragma once

#include "Vertex.h"
#include "Renderer_Utility_Literals.h"

#include <string>
#include <fstream>
#include <assert.h>
#include <iostream>
#include <gl_core_4_4.h>
#include <intrin.h>
#include <glm/vec4.hpp>
#include <glm/mat2x2.hpp>
#include <glm/mat4x2.hpp>

namespace RendererUtility {

	/**
	*	@brief Loads a single text-based file into a string.
	*	@param a_filePath is the path to the file, including the file's extension.
	*	@param a_outputStr is the string reference passed in to output to.
	*	@return void.
	*/
	inline void LoadTextToString(const char* a_filePath, std::string& a_outputStr) {		// Inline to avoid re-definitions
		std::ifstream textFile(a_filePath);

#pragma region Error Handling
		try {
			if (!textFile.is_open()) {
				char errorMsg[256];
				sprintf_s(errorMsg, "ERROR::TEXTFILE::FAILED_TO_OPEN: %s", a_filePath);

				throw std::runtime_error(errorMsg);
			}
		}
		catch (std::exception const& e) { std::cout << "Exception: " << e.what() << std::endl; }

#pragma endregion

		textFile.seekg(0, std::ios::end);		// Go to end of text file
		a_outputStr.reserve(textFile.tellg());	// Allocate memory for string by getting size in bytes of text file by basing it off the location at the end of text file 
		textFile.seekg(0, std::ios::beg);		// Go to beginning of text file

		a_outputStr.assign(std::istreambuf_iterator<char>(textFile), std::istreambuf_iterator<char>());		// Stream text file from beginning to end into a string
	}

	inline void APIENTRY glDebugOutputCallback(unsigned int a_source, unsigned int a_type, unsigned int a_id, unsigned int a_severity, int a_length, const char* a_msg, const void* a_userParam) {
#if ERROR_CHECK_OPENGL
		if (a_id == 131169 || a_id == 131185 || a_id == 131218 || a_id == 131204) return;	// Ignore un-significant error codes to avoid breaking unecessarily

		std::cout << "=====================" << std::endl;
		std::cout << "DEBUG::" << a_id << "::" << a_msg << std::endl;

		// Source handling
		std::cout << "Source: ";

		switch (a_source) {
			case GL_DEBUG_SOURCE_API:				std::cout << "API"; break;
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM:		std::cout << "WINDOW SYSTEM"; break;
			case GL_DEBUG_SOURCE_SHADER_COMPILER:	std::cout << "SHADER COMPILER"; break;
			case GL_DEBUG_SOURCE_THIRD_PARTY:		std::cout << "THIRD PARTY"; break;
			case GL_DEBUG_SOURCE_APPLICATION:		std::cout << "APPLICATION"; break;
			case GL_DEBUG_SOURCE_OTHER:				std::cout << "OTHER"; break;

		} std::cout << std::endl;

		// Type handling
		std::cout << "Type: ";

		switch (a_type) {
			case GL_DEBUG_TYPE_ERROR:				std::cout << "ERROR"; break;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:	std::cout << "DEPRECATED BEHAVIOUR"; break;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:	std::cout << "UNDEFINED BEHAVIOUR"; break;
			case GL_DEBUG_TYPE_PORTABILITY:			std::cout << "PORTABILITY"; break;
			case GL_DEBUG_TYPE_PERFORMANCE:			std::cout << "PERFORMANCE"; break;
			case GL_DEBUG_TYPE_MARKER:				std::cout << "MARKER"; break;
			case GL_DEBUG_TYPE_PUSH_GROUP:			std::cout << "PUSH GROUP"; break;
			case GL_DEBUG_TYPE_POP_GROUP:			std::cout << "POP GROUP"; break;
			case GL_DEBUG_TYPE_OTHER:				std::cout << "OTHER"; break;

		} std::cout << std::endl;

		// Severity handling
		std::cout << "Severity: ";

		switch (a_severity) {
			case GL_DEBUG_SEVERITY_HIGH:			std::cout << "HIGH"; break;
			case GL_DEBUG_SEVERITY_MEDIUM:			std::cout << "MEDIUM"; break;
			case GL_DEBUG_SEVERITY_LOW:				std::cout << "LOW"; break;
			case GL_DEBUG_SEVERITY_NOTIFICATION:	std::cout << "NOTIFICATION"; break;
		} std::cout << std::endl;
		
		std::cout << std::endl;

		// Trigger breakpoint
		__debugbreak();
#endif
	}
	
	/**
	*	@brief Calculate the smoothed tangents for a given triangle (represented by three vertices) and assign the tangent data.
	*	@param a_vert1 is the first unique vertice of the triangle.
	*	@param a_vert2 is the second unique vertice of the triangle.
	*	@param a_vert3 is the third unique vertice of the triangle.
	*	@return void.
	*/
	inline void CalculateNormalTangent(SPRON::Vertex& a_vert1, SPRON::Vertex& a_vert2, SPRON::Vertex& a_vert3) {
		// Define equation variables
		//// World space
		glm::vec4 triEdge1 = a_vert2.pos - a_vert1.pos;
		glm::vec4 triEdge2 = a_vert3.pos - a_vert1.pos;

		//// Tangent space
		glm::vec2 texEdge1 = a_vert2.texCoord - a_vert1.texCoord;
		glm::vec2 texEdge2 = a_vert3.texCoord - a_vert1.texCoord;

		// Take re-arranged tri edge calculation equation in matrix form and extrapolate (calculate) only the tangent components
		float inversion = 1.f / (texEdge1.x * texEdge2.y - texEdge2.x - texEdge1.y);	// Tex edge matrix is inverted in equation, store reality of inversion
		
		glm::vec4 tangent;
		tangent.x = inversion * (texEdge2.y * triEdge1.x - texEdge1.y * triEdge2.x);
		tangent.y = inversion * (texEdge2.y * triEdge1.y - texEdge1.y * triEdge2.y);
		tangent.z = inversion * (texEdge2.y * triEdge1.z - texEdge1.y * triEdge2.z);

		// Smooth tangent normal by normalizing the calculated tangent (normalize tangents so when combined into overall surface normal for tri it ends up averaged)
		tangent = glm::normalize(tangent);

		a_vert1.normalTangent = tangent;
		a_vert2.normalTangent = tangent;
		a_vert3.normalTangent = tangent;
	}
}