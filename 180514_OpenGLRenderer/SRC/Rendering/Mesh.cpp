#include "Mesh.h"
#include "VertexFormat.h"
#include "Material.h"

#include <gl_core_4_4.h>

Mesh::Mesh(const std::vector<float>& a_verts, Material* a_mat, VertexFormat* a_format)
{
	m_rawVerticeData	= a_verts;		// Copy temporary contents into permanent class variable
	m_material			= a_mat;
	m_vertFormat		= a_format;

	// Create vertex buffer and bind to set vertex data
	glGenBuffers(1, &m_vertBufferID);

	glBindBuffer(GL_ARRAY_BUFFER, m_vertBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_rawVerticeData.size(), &m_rawVerticeData[0], GL_STATIC_DRAW);

	/// Pre-defined memory layout attributes
	// NOTE: Stride is based on the overall size of the vertex, e.g. stride of 32 for a vertex containing 8 floats.

	// Position
	m_vertFormat->AddAttribute(*this, 0, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), NULL);

	// Color
	m_vertFormat->AddAttribute(*this, 1, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(4 * sizeof(float)));		// Offset to start in front of position

	// Texture
	m_vertFormat->AddAttribute(*this, 2, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(8 * sizeof(float)));
}

Mesh::~Mesh()
{	
	/// NOTE: Only delete things unique to the mesh, not things that can be shared like materials and formats
	// Delete vertex buffer
	glDeleteBuffers(1, &m_vertBufferID);
}

Material* Mesh::GetMaterial()
{
	return m_material;
}

void Mesh::Draw()
{
	// Load materials, and formatting into openGL before drawing
	glUseProgram(*m_material);
	m_vertFormat->SetAsContext();

	glDrawElements(GL_TRIANGLES, m_vertFormat->GetElementNum(), GL_UNSIGNED_INT, 0);		// Renderer shape hint, number of indices, offset in indice buffer
}
