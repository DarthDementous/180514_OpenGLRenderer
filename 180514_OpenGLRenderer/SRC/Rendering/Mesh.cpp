#include "Mesh.h"
#include "VertexFormat.h"
#include "Material.h"
#include "RenderCamera.h"
#include "Transform.h"
#include "Renderer_Utility_Literals.h"

#include <gl_core_4_4.h>

Mesh::Mesh(const std::vector<float>& a_verts, eMeshType a_type, Material* a_mat, VertexFormat* a_format, Transform* a_transform)
{
	m_rawVerticeData	= a_verts;		// Copy temporary contents into permanent class variable
	m_material			= a_mat;
	m_vertFormat		= a_format;
	m_transform			= a_transform;

	// Create vertex buffer and bind to set vertex data
	glGenBuffers(1, &m_vertBufferID);

	glBindBuffer(GL_ARRAY_BUFFER, m_vertBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_rawVerticeData.size(), &m_rawVerticeData[0], GL_STATIC_DRAW);

	/// Pre-defined memory layout attributes
	// NOTE: Stride is based on the overall size of the vertex, e.g. stride of 32 for a vertex containing 8 floats.

	if (a_type == COLOR) {
		m_vertFormat->AddAttribute(*this, 0, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);							
		m_vertFormat->AddAttribute(*this, 1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(4 * sizeof(float)));	// Offset to start in front of position
	}
	if (a_type == TEXTURE) {
		m_vertFormat->AddAttribute(*this, 0, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		m_vertFormat->AddAttribute(*this, 2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(4 * sizeof(float)));
	}
	if (a_type == TEXTURE_NORMAL) {
		m_vertFormat->AddAttribute(*this, 0, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)0);
		m_vertFormat->AddAttribute(*this, 2, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(4 * sizeof(float)));
		m_vertFormat->AddAttribute(*this, 3, 4, GL_FLOAT, GL_TRUE, 10 * sizeof(float), (void*)(6 * sizeof(float)));
	}
	if (a_type == COLOR_TEXTURE) {
		m_vertFormat->AddAttribute(*this, 0, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)0);
		m_vertFormat->AddAttribute(*this, 1, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(4 * sizeof(float)));
		m_vertFormat->AddAttribute(*this, 2, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(8 * sizeof(float)));	
	}
	if (a_type == COLOR_TEXTURE_NORMAL) {
		m_vertFormat->AddAttribute(*this, 0, 4, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);						// Position
		m_vertFormat->AddAttribute(*this, 1, 4, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(4 * sizeof(float)));	// Color
		m_vertFormat->AddAttribute(*this, 2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));	// Texture coords
		m_vertFormat->AddAttribute(*this, 3, 4, GL_FLOAT, GL_TRUE, 14 * sizeof(float), (void*)(10 * sizeof(float)));	// Vertex normal
	}

}

Mesh::~Mesh()
{	
	/// NOTE: Only delete things unique to the mesh, not things that can be shared like materials and formats
	// Delete vertex buffer
	glDeleteBuffers(1, &m_vertBufferID);

	// Clean up dynamically allocated memory
	delete m_transform;
}

Material* Mesh::GetMaterial()
{
	return m_material;
}

Transform * Mesh::GetTransform()
{
	return m_transform;
}

/**
*	@brief Visually render mesh based on its vertices, transform and render camera.
*	@param a_camera is the camera to render to.
*/
void Mesh::Draw(RenderCamera* a_camera)
{
	assert(a_camera && "ERROR::MESH::NULL_CAMERA");

	// Update rendering transforms
	m_material->SetMat4("modelTransform", m_transform->GetMatrix());
	m_material->SetMat4("viewTransform", a_camera->CalculateView());
	m_material->SetMat4("projectionTransform", a_camera->GetProjection());

	// Update lighting data
	m_material->SetVec4("viewerPos", glm::vec4(a_camera->GetTransform()->GetPosition(), 1));

	// Load materials, and formatting into openGL before drawing
	glUseProgram(*m_material);
	m_vertFormat->SetAsContext();

	// Determine draw method from vertex format
	unsigned int indiceNum = m_vertFormat->GetElementNum();

	if (indiceNum > 1) {	// Mesh has preset draw format
		glDrawElements(GL_TRIANGLES, m_vertFormat->GetElementNum(), GL_UNSIGNED_INT, 0);		// Renderer shape hint, number of indices, offset in indice buffer
	}
	else {					// Mesh has no preset draw format
		unsigned int vertNum = sizeof(float) * m_rawVerticeData.size() / (sizeof(float) * 10);	// Divide array size by vertex stride to get number of vertices
		glDrawArrays(GL_TRIANGLES, 0, vertNum);
	}
}
