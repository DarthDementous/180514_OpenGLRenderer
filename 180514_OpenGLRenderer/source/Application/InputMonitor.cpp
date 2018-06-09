#include "InputMonitor.h"
#include "Renderer_Utility_Literals.h"
#include "ShaderWrapper.h"

#include <GLFW\glfw3.h>
#include <iostream>
#include <imgui.h>

namespace SPRON {

	/// Static initialisation
	InputMonitor* InputMonitor::m_singleton = nullptr;

	InputMonitor::InputMonitor()
	{
		// Set initial last cursor position
		m_lastCursorPos;
		glfwGetCursorPos(glfwGetCurrentContext(), &m_lastCursorPos.x, &m_lastCursorPos.y);
	}

	InputMonitor::~InputMonitor()
	{

	}

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
		if (glfwGetMouseButton(glfwGetCurrentContext(), a_mouseButtonCode) == GLFW_PRESS) {
			return true;
		}
		else {
			return false;
		}
	}

	bool InputMonitor::GetMouseButtonUp(int a_mouseButtonCode)
	{
		// Mouse button is up
		if (glfwGetMouseButton(glfwGetCurrentContext(), a_mouseButtonCode) == GLFW_RELEASE) {
			return true;
		}
		else {
			return false;
		}
	}

	glm::vec2 InputMonitor::GetMouseDeltaXY()
	{
		// Get cursor position for this frame
		glm::tvec2<double> currentCursorPos = glm::vec2();
		glfwGetCursorPos(glfwGetCurrentContext(), &currentCursorPos.x, &currentCursorPos.y);

		// Calculate difference between current position and previous position
		glm::vec2 mouseDelta = currentCursorPos - m_lastCursorPos;

		// Set current position to previous position for future calculations
		m_lastCursorPos = currentCursorPos;

		return mouseDelta;
	}
}



