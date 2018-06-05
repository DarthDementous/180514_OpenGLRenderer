#include <iostream>
#include "RendererProgram.h"

int main() {

	RendererProgram* program = new RendererProgram();

	program->Run("OpenGL Rendering Program", 1280, 720);

	return EXIT_SUCCESS;
}