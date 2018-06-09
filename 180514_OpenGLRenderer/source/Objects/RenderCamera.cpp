#include "RenderCamera.h"
#include "Transform.h"
#include "InputMonitor.h"

#include <glm/ext.hpp>
#include <GLFW\glfw3.h>
#include <iostream>

namespace SPRON {

	RenderCamera::RenderCamera(float a_maxSpeed, float a_maxRotSpeed) : m_maxMoveSpeed(a_maxSpeed), m_maxRotSpeed(a_maxRotSpeed)
	{
		/// Variable initialisation
		m_cameraTransform = new Transform();
	}

	RenderCamera::~RenderCamera()
	{
	}

	glm::mat4 RenderCamera::CalculateProjectionView()
	{
		glm::vec3 target = m_cameraTransform->GetPosition() + m_cameraTransform->Forward();
		glm::mat4 viewMatrix = glm::lookAt(m_cameraTransform->GetPosition(), target, glm::vec3(0, 1, 0));	// Create view matrix looking in front of the camera according to its transform

		return m_projectionMatrix * viewMatrix;
	}

	glm::mat4 RenderCamera::CalculateView()
	{
		glm::vec3 target = m_cameraTransform->GetPosition() + m_cameraTransform->Forward();
		glm::mat4 viewMatrix = glm::lookAt(m_cameraTransform->GetPosition(), target, glm::vec3(0, 1, 0));	// Create view matrix looking in front of the camera according to its transform

		return viewMatrix;
	}

	glm::mat4 RenderCamera::GetProjection()
	{
		return m_projectionMatrix;
	}

	SPRON::Transform * RenderCamera::GetTransform()
	{
		return m_cameraTransform;
	}

	void RenderCamera::Update(float a_dt)
	{
		InputMonitor* input = InputMonitor::GetInstance();

		///Keyboard input
		if (input->GetMouseButtonDown(GLFW_MOUSE_BUTTON_2)) {	// Right mouse button is held down
			if (input->GetKeyDown(GLFW_KEY_W)) {		// User wants to move camera FORWARD
				m_cameraTransform->Translate(m_cameraTransform->Forward() * m_maxMoveSpeed * a_dt);
			}

			if (input->GetKeyDown(GLFW_KEY_S)) {		// User wants to move camera BACKWARDS
				m_cameraTransform->Translate(-m_cameraTransform->Forward() * m_maxMoveSpeed * a_dt);
			}

			if (input->GetKeyDown(GLFW_KEY_A)) {		// User wants to move camera LEFT
				m_cameraTransform->Translate(m_cameraTransform->Left() * m_maxMoveSpeed * a_dt);
			}

			if (input->GetKeyDown(GLFW_KEY_D)) {		// User wants to move camera RIGHT
				m_cameraTransform->Translate(-m_cameraTransform->Left() * m_maxMoveSpeed * a_dt);
			}
		}

		/// Mouse input
		glm::vec2 mouseProj = input->GetMouseDeltaXY();									// Calculate delta regardless to avoid cursor jumping

		// Drag mode
		if (mouseProj.y != 0 && input->GetMouseButtonDown(GLFW_MOUSE_BUTTON_2)) {		// Mouse has moved and right click is held down
			// Hide cursor and clamp to screen while in drag mode
			glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

			m_currentPitch += mouseProj.y * a_dt * m_maxRotSpeed;
			m_currentYaw += mouseProj.x * a_dt * m_maxRotSpeed;

			// Clamp pitch to avoid gimbal lock
			m_currentPitch = glm::clamp(m_currentPitch, -70.f, 70.f);

			m_cameraTransform->SetRotation(glm::vec3(glm::radians(m_currentPitch), glm::radians(-m_currentYaw), 0));
		}
		// Released mode
		else if (input->GetMouseButtonUp(GLFW_MOUSE_BUTTON_2)) {						// Right mouse button is not held down
			// Show normal cursor
			glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}

	}

	void RenderCamera::SetProjection(float a_fov, float a_aspectRatio, float a_near, float a_far)
	{
		m_projectionMatrix = glm::perspective(a_fov, a_aspectRatio, a_near, a_far);
	}
}