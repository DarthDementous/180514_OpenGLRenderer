#pragma once

#include <vector>

class VertexFormat {
public:
	VertexFormat();
	VertexFormat(const std::vector<unsigned int>& a_indices);
	~VertexFormat();

	void AddAttribute(unsigned int a_vertBufferID, unsigned int a_attributeLocation, unsigned int a_elementNum, unsigned int a_elementType, bool a_isNormalised, int a_stride, const void* a_offset);
	void SetAsContext();

	unsigned int GetElementNum() { return (unsigned int)m_indiceData.size(); }

	operator unsigned int() { return m_ID; }	// Allow class to be used in parameters of openGL functions
protected:
private:
	unsigned int m_ID;				// OpenGL vertex array object identifier
	unsigned int m_elementBufferID;

	std::vector<unsigned int> m_indiceData;		// Keep track of vertex draw order to ensure it stays in scope for openGL
};