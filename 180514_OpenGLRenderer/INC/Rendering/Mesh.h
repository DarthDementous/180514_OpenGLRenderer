#pragma once

#include <vector>

class RenderCamera;

class VertexFormat;
class Material;
class Transform;

enum eMeshType {
	COLOR,
	TEXTURE,
	TEXTURE_NORMAL,
	COLOR_TEXTURE,
	COLOR_TEXTURE_NORMAL
};


class Mesh {
public:
	Mesh() {}
	Mesh(const std::vector<float>& a_verts, eMeshType a_type, Material* a_mat, VertexFormat* a_format, Transform* a_transform);
	~Mesh();

	Material* GetMaterial();
	Transform* GetTransform();
	std::vector<float> GetVerticeData() { return m_rawVerticeData; }

	operator unsigned int() { return m_vertBufferID; }

	void Draw(RenderCamera* a_camera);
protected:
private:
	unsigned int m_vertBufferID;	// Hold onto vertex buffer identifier

	Material*		m_material;		// What shaders to apply to the mesh
	VertexFormat*	m_vertFormat;	// How vertex data is interpreted

	std::vector<float> m_rawVerticeData;	// Keep track of vertex data so memory isn't freed until Mesh is deleted

	Transform* m_transform;			// Transform information in global space
};