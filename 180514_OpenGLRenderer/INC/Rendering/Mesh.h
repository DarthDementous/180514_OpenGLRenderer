#pragma once

#include <vector>

class VertexFormat;
class Material;

class Mesh {
public:
	Mesh(std::vector<float>& a_verts, Material* a_mat, VertexFormat* a_format);
	~Mesh();

	operator unsigned int() { return m_vertBufferID; }

	void Draw();
protected:
private:
	unsigned int m_vertBufferID;	// Hold onto vertex buffer identifier

	Material*		m_material;		// What shaders to apply to the mesh
	VertexFormat*	m_vertFormat;	// How vertex data is interpreted

	std::vector<float> m_rawVerticeData;	// Keep track of vertex data so memory isn't freed until Mesh is deleted
};