#pragma once

#include <Program.h>
#include <glm/mat4x4.hpp>

class RenderCamera;
class Transform;

class RendererProgram : public Program {
public:
	RendererProgram();
	virtual ~RendererProgram();
protected:
	virtual int		Startup();
	virtual void	Shutdown();

	virtual void Update(float a_dt);
	virtual void Render();
private:
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;

	RenderCamera* mainCamera;

	Transform* sphereTransform;
};