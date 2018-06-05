#include "InputMonitor.h"
#include "Renderer_Utility_Literals.h"

#include <GLFW\glfw3.h>
#include <iostream>


/// Static initialisation
InputMonitor* InputMonitor::m_singleton = nullptr;

InputMonitor * InputMonitor::GetInstance()
{
	if (!m_singleton) {
		m_singleton = new InputMonitor();
	}

	return m_singleton;
}

bool InputMonitor::GetKeyDown(int a_keyCode)
{
	// Key is pressed this frame
	if (glfwGetKey(glfwGetCurrentContext(), a_keyCode) == GLFW_PRESS) {
		return true;
	}
	else {
		return false;
	}
}

bool InputMonitor::GetMouseButtonDown(int a_mouseButtonCode)
{
	// Mouse button is held down
	if (glfwGetMouseButton(glfwGetCurrentContext(), a_mouseButtonCode)) {
		return true;
	}
	else {
		return false;
	}
}

glm::vec2 InputMonitor:: GetMouseDeltaXY()
{
	glm::tvec2<double> currentCursorPos = glm::vec2();
	glfwGetCursorPos(glfwGetCurrentContext(), &currentCursorPos.x, &currentCursorPos.y);

	glm::vec2 mouseDelta = currentCursorPos - m_lastCursorPos;

	m_lastCursorPos = currentCursorPos;

	return mouseDelta;
}



InputMonitor::InputMonitor()
{
	// Set initial last cursor position
	m_lastCursorPos = glm::vec2();
	glfwGetCursorPos(glfwGetCurrentContext(), &m_lastCursorPos.x, &m_lastCursorPos.y);

	// Set cursor mode
	glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

InputMonitor::~InputMonitor()
{

}