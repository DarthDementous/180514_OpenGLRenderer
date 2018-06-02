#pragma once

#include <Program.h>
#include <glm/mat4x4.hpp>
#include <vector>

class Mesh;
class RenderCamera;
class Transform;
class TextureWrapper;
class PhongLight_Dir;
class PhongLight_Point;
class PhongLight_Spot;

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

	Mesh* rectMesh;
	Mesh* rhombusMesh;
	Mesh* lightRepMesh;
	std::vector<Mesh*> cubeMeshes;

	TextureWrapper* wallTex;
	TextureWrapper* faceTex;
	TextureWrapper* lightTex;
	TextureWrapper*	crateTex;
	TextureWrapper* crateSpecularTex;

	PhongLight_Dir*		dirLight;
	PhongLight_Point*	ptLight1;
	PhongLight_Point*	ptLight2;
	PhongLight_Spot*	spotLight;
};