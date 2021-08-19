// Buffer.cpp
// Spontz Demogroup

#include "main.h"
#include "core/renderer/Buffer.h"

namespace Phoenix {

	// Creation of a Dynamic buffer (data can be uploaded with SetData)
	VertexBuffer::VertexBuffer(size_t size)
	{
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferStorage(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_STORAGE_BIT);
		// In case we want to use DSA, we can remove glBindBuffer and glBufferStorage and use this
		// glNamedBufferStorage(m_RendererID, size, nullptr, GL_DYNAMIC_STORAGE_BIT);
	}

	// Creation of a static buffer (data will not be uploaded ever again)
	VertexBuffer::VertexBuffer(const void* data, size_t size)
	{
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferStorage(GL_ARRAY_BUFFER, size, data, 0);
		// In case we want to use DSA, we can remove glBindBuffer and glBufferStorage and use this
		// glNamedBufferStorage(m_RendererID, size, data, 0);
	}

	VertexBuffer::~VertexBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void VertexBuffer::Bind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	}

	void VertexBuffer::Unbind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void VertexBuffer::SetData(const void* data, size_t size)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
		// In case we want to use DSA, we can remove glBindBuffer and glBufferSubData and use this
		// glNamedBufferSubData(m_RendererID, 0, size, data);
	}

	// Index Buffer
	IndexBuffer::IndexBuffer(const uint32_t* indices, uint32_t count)
		:
		m_Count(count)
	{
		glCreateBuffers(1, &m_RendererID);
		// GL_ELEMENT_ARRAY_BUFFER is not valid without an actively bound VAO
		// Binding with GL_ARRAY_BUFFER allows the data to be loaded regardless of VAO state.
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferStorage(GL_ARRAY_BUFFER, size_t(count) * sizeof(uint32_t), indices, 0);
		// In case we want to use DSA, we can remove glBindBuffer and glBufferStorage and use this
		// glNamedBufferStorage(m_RendererID, count * sizeof(uint32_t), indices, 0);
	}

	IndexBuffer::~IndexBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void IndexBuffer::Bind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	}

	void IndexBuffer::Unbind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}
