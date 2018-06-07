#pragma once

#include <glm/vec2.hpp>

struct GLFWwindow;

namespace SPRON {
	class InputMonitor {
	public:
		static InputMonitor* GetInstance();

		bool GetKeyDown(int a_keyCode);
		bool GetMouseButtonDown(int a_mouseButtonCode);
		bool GetMouseButtonUp(int a_mouseButtonCode);

		glm::vec2 GetMouseDeltaXY();
	protected:
	private:
		InputMonitor();
		~InputMonitor();

		static InputMonitor* m_singleton;

		glm::tvec2<double> m_lastCursorPos;
	};
}