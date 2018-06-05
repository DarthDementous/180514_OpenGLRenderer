#pragma once

#include <vector>
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

class RenderCamera;

class VertexFormat;
class ShaderWrapper;
class TextureWrapper;
class Transform;
class PhongLight;

#pragma region Structs
// Holds onto information to be passed into the forward rendering light shader programs
struct Material {
	Material(const glm::vec4& a_ambient = glm::vec4(1.f), 
		const glm::vec4& a_diffuse = glm::vec4(1.f), 
		const glm::vec4& a_specular = glm::vec4(1.f), 
		float a_shininess = 32.f, TextureWrapper* a_diffuseMap = nullptr, TextureWrapper* a_specularMap = nullptr) {

		ambientColor	= a_ambient;
		diffuseColor	= a_diffuse;
		specular		= a_specular;

		shininessCoefficient = a_shininess;

		diffuseMap	= a_diffuseMap;
		specularMap = a_specularMap;
	}
	~Material() {}

	glm::vec4 ambientColor;
	glm::vec4 diffuseColor;
	glm::vec4 specular;

	float shininessCoefficient;

	TextureWrapper* diffuseMap = nullptr;		// 'Texture' of the object
	TextureWrapper* specularMap = nullptr;
};

// Holds onto data contained within the vertex for readable usage with openGL code
struct Vertex {
	Vertex(const glm::vec4& a_pos, const glm::vec2& a_coord, const glm::vec4& a_normal) {
		pos			= a_pos;
		texCoord	= a_coord;
		normal		= a_normal;
	}

	glm::vec4 pos;
	glm::vec2 texCoord;
	glm::vec4 normal;
};

#pragma endregion 

class Mesh {
public:
	Mesh() {}
	Mesh(const std::vector<Vertex>& a_verts, VertexFormat* a_format, Transform* a_transform,
		Material a_material = Material());		// Set default material values if not defined in constructor
	~Mesh();

	Material& GetMaterial();
	Transform* GetTransform();
	std::vector<Vertex> GetVerticeData() { return m_rawVerticeData; }

	operator unsigned int() { return m_vertBufferID; }

	void Draw(RenderCamera* a_camera, 
		std::vector<PhongLight*> a_lights, 
		const glm::vec4& a_globalAmbient, ShaderWrapper* a_ambientPass, 
		ShaderWrapper* a_directionalPass, ShaderWrapper* a_pointPass, ShaderWrapper* a_spotPass);

	void SetMaterial(Material a_material);
protected:
	void Render(ShaderWrapper* a_shaderProgram);
private:
	unsigned int m_vertBufferID;	// Hold onto vertex buffer identifier

	Material m_material;

	VertexFormat*	m_vertFormat;			// How vertex data is interpreted

	std::vector<Vertex> m_rawVerticeData;	// Keep track of vertex data so memory isn't freed until Mesh is deleted

	Transform* m_parentTransform;	// Hold onto parent transform so that changes made to it will apply to all of its child meshes
	Transform* m_transform;			// Transform information in global space
};