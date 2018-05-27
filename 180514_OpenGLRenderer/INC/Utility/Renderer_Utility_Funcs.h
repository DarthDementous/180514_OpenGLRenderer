#pragma once

#include <string>
#include <fstream>
#include <assert.h>
#include <iostream>
#include <gl_core_4_4.h>
#include <intrin.h>

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
	}
}