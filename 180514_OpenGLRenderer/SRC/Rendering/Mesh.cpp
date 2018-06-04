#include "Mesh.h"
#include "VertexFormat.h"
#include "ShaderWrapper.h"
#include "RenderCamera.h"
#include "Transform.h"
#include "Renderer_Utility_Literals.h"
#include "Light\PhongLight_Dir.h"
#include "Light\PhongLight_Point.h"
#include "Light\PhongLight_Spot.h"

#include <gl_core_4_4.h>

Mesh::Mesh(const std::vector<Vertex>& a_verts, VertexFormat* a_format, Transform* a_transform, Material a_material)
{
	m_rawVerticeData	= a_verts;		// Copy temporary contents into permanent class variable
	m_material			= a_material;
	m_vertFormat		= a_format;
	m_transform			= a_transform;

	// Create vertex buffer and bind to set vertex data
	glGenBuffers(1, &m_vertBufferID);

	glBindBuffer(GL_ARRAY_BUFFER, m_vertBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * m_rawVerticeData.size(), &m_rawVerticeData[0], GL_STATIC_DRAW);

	/// Pre-defined memory layout attributes
	// NOTE: Stride is based on the overall size of the vertex, e.g. stride of 32 for a vertex containing 8 floats.
	
	//// Vertex Position
	m_vertFormat->AddAttribute(*this, 0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	//// Vertex Texture Coords
	m_vertFormat->AddAttribute(*this, 1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));	// Use offsetof macro to get the point in which the tex coordinates data starts

	//// Vertex Normal
	m_vertFormat->AddAttribute(*this, 2, 4, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
}

Mesh::~Mesh()
{	
	/// NOTE: Only delete things unique to the mesh, not things that can be shared like materials and formats
	// Delete vertex buffer
	glDeleteBuffers(1, &m_vertBufferID);

	// Clean up dynamically allocated memory
	delete m_transform;
}

Material Mesh::GetMaterial()
{
	return m_material;
}

Transform * Mesh::GetTransform()
{
	return m_transform;
}

/**
*	@brief Visually render mesh based on its vertices, transform, and render camera and lights.
*	NOTE: If a light shader is set to nullptr then that pass will not be performed.
*	O(L) complexity where L = number of lights
*	@param a_camera is the camera to render to.
*	@param a_lights is the vector of lights to take lighting information from.
*	@param a_globalAmbient is the global ambience to take into account when performing the ambient lighting pass.
*	@param a_ambientPass is the shader program to use during the ambient lighting pass.
*	@param a_directionalPass is the shader program to use during the directional lighting pass.
*	@param a_pointPass is the shader program to use during the point lighting pass.
*/
void Mesh::Draw(RenderCamera* a_camera, std::vector<PhongLight*> a_lights, 
	const glm::vec4& a_globalAmbient, ShaderWrapper* a_ambientPass, 
	ShaderWrapper* a_directionalPass, ShaderWrapper* a_pointPass, ShaderWrapper* a_spotPass)
{
	assert(a_camera && "ERROR::MESH::NULL_CAMERA");
	
	/// Set global rendering data
	if (a_ambientPass) {
		//// Ambient pass (only performed once)
		a_ambientPass->SetMat4("modelTransform", m_transform->GetMatrix());
		a_ambientPass->SetMat4("viewTransform", a_camera->CalculateView());
		a_ambientPass->SetMat4("projectionTransform", a_camera->GetProjection());
		
		// Set lighting data
		a_ambientPass->SetVec4("ambient", a_globalAmbient * m_material.ambientColor);	// Combine global ambience with material ambience
		a_ambientPass->SetTexture("texSample", m_material.diffuseMap);
		a_ambientPass->SetBool("useTex", (m_material.diffuseMap ? true : false));

		// Perform render pass
		Render(a_ambientPass);
	}

	if (a_directionalPass) {
		// Set render transforms
		a_directionalPass->SetMat4("modelTransform", m_transform->GetMatrix());
		a_directionalPass->SetMat4("viewTransform", a_camera->CalculateView());
		a_directionalPass->SetMat4("projectionTransform", a_camera->GetProjection());

		// Set material data
		a_directionalPass->SetMaterial("material", m_material);

		// Set additional data
		a_directionalPass->SetVec4("viewerPos", glm::vec4(a_camera->GetTransform()->GetPosition(), 1));
	}

	if (a_pointPass) {
		// Set render transforms
		a_pointPass->SetMat4("modelTransform", m_transform->GetMatrix());
		a_pointPass->SetMat4("viewTransform", a_camera->CalculateView());
		a_pointPass->SetMat4("projectionTransform", a_camera->GetProjection());

		// Set material data
		a_pointPass->SetMaterial("material", m_material);

		// Set additional data
		a_pointPass->SetVec4("viewerPos", glm::vec4(a_camera->GetTransform()->GetPosition(), 1));
	}

	if (a_spotPass) {
		// Set render transforms
		a_spotPass->SetMat4("modelTransform", m_transform->GetMatrix());
		a_spotPass->SetMat4("viewTransform", a_camera->CalculateView());
		a_spotPass->SetMat4("projectionTransform", a_camera->GetProjection());

		// Set material data
		a_spotPass->SetMaterial("material", m_material);

		// Set additional data
		a_spotPass->SetVec4("viewerPos", glm::vec4(a_camera->GetTransform()->GetPosition(), 1));
	}

	/// Forward rendering
	// Activate blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);	// Take existing frag color *1 and add it onto the new frag color *1

	glDepthMask(false);				// Disable writing to depth buffer
	glDepthFunc(GL_EQUAL);			// Only tries to adds on the new pixel with the lighting if it has the same depth value as the pixel nearest to the screen
	
	// Loop through each light and apply appropriate lighting passes
	for (int i = 0; i < a_lights.size(); ++i) {

		//// Directional pass
		if (a_directionalPass && a_lights[i]->GetType() == DIRECTIONAL_LIGHT) {		// Directional lighting shader program provided and light is directional
			
			PhongLight_Dir* dirLight = (PhongLight_Dir*)a_lights[i];

			// Set lighting data
			a_directionalPass->SetDirectionalLight("dirLight", dirLight);

			// Perform render pass
			Render(a_directionalPass);
		}

		//// Point pass
		if (a_pointPass && a_lights[i]->GetType() == POINT_LIGHT) {

			PhongLight_Point* ptLight = (PhongLight_Point*)a_lights[i];

			// Set lighting data
			a_pointPass->SetPointLight("ptLight", ptLight);

			// Perform render pass
			Render(a_pointPass);
		}

		//// Spot pass
		if (a_spotPass && a_lights[i]->GetType() == SPOT_LIGHT) {

			PhongLight_Spot* spotLight = (PhongLight_Spot*)a_lights[i];

			// Set lighting data
			a_spotPass->SetSpotLight("spotLight", spotLight);

			// Perform render pass
			Render(a_spotPass);
		}
	}

	glDepthFunc(GL_LESS);			// Set back to default depth function
	glDepthMask(true);				// Re-enable writing to depth buffer

	// De-activate blending
	glDisable(GL_BLEND);	
}

void Mesh::SetMaterial(Material a_material)
{
	m_material = a_material;
}

/**
*	@brief Draw vertices with bound shader program.
*	NOTE: This can be used multiple times with forward rendering light shaders to create an overall blend with multiple render passes.
*	@param a_shaderProgram is the shader program to use to render the mesh with.
**/
void Mesh::Render(ShaderWrapper * a_shaderProgram)
{
	// Bind shader program
	glUseProgram(*a_shaderProgram);

	// Bind vertex formatting
	m_vertFormat->SetAsContext();

	// Determine render method from vertex format
	unsigned int indiceNum = m_vertFormat->GetElementNum();

	if (indiceNum > 1) {	// Mesh has preset draw format
		glDrawElements(GL_TRIANGLES, m_vertFormat->GetElementNum(), GL_UNSIGNED_INT, 0);		// Renderer shape hint, number of indices, offset in indice buffer
	}
	else {					// Mesh has no preset draw format
		unsigned int vertNum = sizeof(Vertex) * m_rawVerticeData.size() / (sizeof(Vertex));		// Divide array size by vertex stride to get number of vertices
		glDrawArrays(GL_TRIANGLES, 0, vertNum);
	}

}
