#pragma once

#include <glm/mat4x4.hpp>

class Transform;

class RenderCamera {
public:
	RenderCamera(float a_maxSpeed = 10, float a_maxRotSpeed = 5);
	~RenderCamera();

	void Update(float a_dt);

	void SetProjection(float a_fov, float a_aspectRatio, float a_near, float a_far);

	glm::mat4 CalculateProjectionView();
	glm::mat4 CalculateView();
	glm::mat4 GetProjection();

	Transform* GetTransform();

protected:
private:
	float m_maxMoveSpeed;
	float m_maxRotSpeed;
	float m_currentPitch;
	float m_currentYaw;

	glm::mat4 m_projectionMatrix;

	Transform* m_cameraTransform;
};