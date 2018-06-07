#include "VertexFormat.h"

#include <gl_core_4_4.h>

namespace SPRON {

	VertexFormat::VertexFormat()
	{
	}

	VertexFormat::VertexFormat(const std::vector<unsigned int>& a_indices)
	{
		m_indiceData = a_indices;

		// Initialise vertex array on GPU
		glGenVertexArrays(1, &m_ID);

		// Bind so that indice buffer info is stored in vertex array object
		SetAsContext();

		// Set draw order of vertices by creating and holding onto an element buffer
		glGenBuffers(1, &m_elementBufferID);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementBufferID);	// NOTE: Need to bind buffer before data can be set
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * m_indiceData.size(), &m_indiceData[0], GL_STATIC_DRAW);	// TODO: Allow user to specify multiple draw types instead of just static
	}

	VertexFormat::~VertexFormat()
	{
		// Clean up vertex array
		glDeleteVertexArrays(1, &m_ID);

		// Clean up element buffer
		glDeleteBuffers(1, &m_elementBufferID);
	}

	/**
	*	@brief Bind and activate vertex attribute to current bound vertex buffer to define how to read certain vertex types.
	*	@param a_vertBufferID is the id of the vertice buffer to bind this memory layout to.
	*	@param a_attributeLocation is the place in memory to find the specified attribute.
	*	@param a_elementNum is the number of elements in the vertex.
	*	@param a_elementType is the gl enum corresponding to the data type of the vertex elements.
	*	@param a_isNormalised specifies whether to normalise the values in the vertex.
	*	@param a_stride is how big in bytes each vertex is.
	*	@param a_offset is how much space in between each vertex in a buffer there is.
	*	@return void.
	*/
	void VertexFormat::AddAttribute(unsigned int a_vertBufferID, unsigned int a_attributeLocation, unsigned int a_elementNum, unsigned int a_elementType, bool a_isNormalised, int a_stride, const void * a_offset)
	{
		/* NOTE: A bound vertex array will keep track of all glBindBuffers and glVertexAttribPointers, and the glVertexAttribPointers will keep track of
		the currently bound vertex buffer object, meaning that the vertex buffer object must be defined and bound before the formatting is set.*/
		glBindBuffer(GL_ARRAY_BUFFER, a_vertBufferID);
		SetAsContext();

		glVertexAttribPointer(a_attributeLocation, a_elementNum, a_elementType, a_isNormalised, a_stride, a_offset);
		glEnableVertexAttribArray(a_attributeLocation);
	}

	/**
	*	@brief Bind vertex array as current context so that creation of attribute pointers and indice buffers are saved to it.
	*	Any glBindBuffer calls will be stored in the vertex array that has the current context.
	*	CAUTION: Unbind calls are also stored so do not unbind element array buffer before unbinding vertex array or it won't have a draw order configured.
	*	NOTE: Will replace any previously bound vertex array.
	*	@return void.
	*/
	void VertexFormat::SetAsContext()
	{
		glBindVertexArray(*this);
	}
}