// VertexArray.cpp
// Spontz Demogroup

#include "main.h"
#include "VertexArray.h"

namespace Phoenix {

	VertexArray::VertexArray()
	{
		glCreateVertexArrays(1, &m_rendererID);
	}

	VertexArray::~VertexArray()
	{
		if (m_rendererID != 0)
			glDeleteVertexArrays(1, &m_rendererID);
	}

	void VertexArray::bind() const
	{
		glBindVertexArray(m_rendererID);
	}

	void VertexArray::unbind() const
	{
		glBindVertexArray(0);
	}

	void VertexArray::AddVertexBuffer(SP_VertexBuffer const& spVertexBuffer)
	{
		glBindVertexArray(m_rendererID);
		spVertexBuffer->Bind();
		const auto& layout = spVertexBuffer->GetLayout();

		for (const auto& element : layout)
		{
			switch (element.Type)
			{
			case ShaderDataType::Float:
			case ShaderDataType::Float2:
			case ShaderDataType::Float3:
			case ShaderDataType::Float4:
			{
				glEnableVertexAttribArray(m_vertexBufferIndex);
				glVertexAttribPointer(m_vertexBufferIndex,
					element.DataType.elementCount,
					element.DataType.GLBaseType,
					element.Normalized ? GL_TRUE : GL_FALSE,
					layout.GetStride(),
					(const void*)element.Offset);
				m_vertexBufferIndex++;
				break;
			}
			case ShaderDataType::Int:
			case ShaderDataType::Int2:
			case ShaderDataType::Int3:
			case ShaderDataType::Int4:
			case ShaderDataType::UInt:
			case ShaderDataType::UInt2:
			case ShaderDataType::UInt3:
			case ShaderDataType::UInt4:
			case ShaderDataType::Bool:
			{
				glEnableVertexAttribArray(m_vertexBufferIndex);
				glVertexAttribIPointer(m_vertexBufferIndex,
					element.DataType.elementCount,
					element.DataType.GLBaseType,
					layout.GetStride(),
					(const void*)element.Offset);
				m_vertexBufferIndex++;
				break;
			}
			case ShaderDataType::Mat3:
			case ShaderDataType::Mat4:
			{
				uint8_t count = element.DataType.elementCount;
				for (uint8_t i = 0; i < count; i++)
				{
					glEnableVertexAttribArray(m_vertexBufferIndex);
					glVertexAttribPointer(m_vertexBufferIndex,
						count,
						element.DataType.GLBaseType,
						element.Normalized ? GL_TRUE : GL_FALSE,
						layout.GetStride(),
						(const void*)(element.Offset + sizeof(float) * count * i));
					glVertexAttribDivisor(m_vertexBufferIndex, 1);
					m_vertexBufferIndex++;
				}
				break;
			}
			default:
				Logger::error("Unknown ShaderDataType");
			}
		}

		m_vertexBuffers.push_back(spVertexBuffer);
		glBindVertexArray(0);
	}

	// DSA Approach, not working on Intel graphics card: https://community.khronos.org/t/data-alignment-in-shader/106560
#ifdef DSA
	void VertexArray::AddVertexBuffer(VertexBuffer* vertexBuffer)
	{
		const auto& layout = vertexBuffer->GetLayout();

		glVertexArrayVertexBuffer(m_RendererID, 0, vertexBuffer->GetBufferID(), 0, layout.GetStride());

		for (const auto& element : layout)
		{
			switch (element.Type)
			{
			case ShaderDataType::Float:
			case ShaderDataType::Float2:
			case ShaderDataType::Float3:
			case ShaderDataType::Float4:
			{
				glEnableVertexArrayAttrib(m_RendererID, m_VertexBufferIndex);
				glVertexArrayAttribFormat(m_RendererID, m_VertexBufferIndex,
					element.DataType.elementCount,
					element.DataType.GLBaseType,
					element.Normalized ? GL_TRUE : GL_FALSE,
					element.Offset);
				glVertexArrayAttribBinding(m_RendererID, m_VertexBufferIndex, 0);	// remove Binding hardcoded to 0

				m_VertexBufferIndex++;
				break;
			}
			case ShaderDataType::Int:
			case ShaderDataType::Int2:
			case ShaderDataType::Int3:
			case ShaderDataType::Int4:
			case ShaderDataType::UInt:
			case ShaderDataType::UInt2:
			case ShaderDataType::UInt3:
			case ShaderDataType::UInt4:
			case ShaderDataType::Bool:
			{
				glEnableVertexArrayAttrib(m_RendererID, m_VertexBufferIndex);
				glVertexArrayAttribIFormat(m_RendererID, m_VertexBufferIndex,
					element.DataType.elementCount,
					element.DataType.GLBaseType,
					element.Offset);

				glVertexArrayAttribBinding(m_RendererID, m_VertexBufferIndex, 0);	// remove Binding hardcoded to 0

				m_VertexBufferIndex++;
				break;
			}
			case ShaderDataType::Mat3:
			case ShaderDataType::Mat4:
			{
				uint8_t count = element.DataType.elementCount;
				for (uint8_t i = 0; i < count; i++)
				{
					glEnableVertexArrayAttrib(m_RendererID, m_VertexBufferIndex);
					glVertexArrayAttribFormat(m_RendererID, m_VertexBufferIndex,
						element.DataType.elementCount,
						element.DataType.GLBaseType,
						element.Normalized ? GL_TRUE : GL_FALSE,
						element.Offset + sizeof(float) * count * i);
					glVertexArrayBindingDivisor(m_RendererID, m_VertexBufferIndex, 1);
					glVertexArrayAttribBinding(m_RendererID, m_VertexBufferIndex, 0);	// remove Binding hardcoded to 0

					m_VertexBufferIndex++;
				}
				break;
			}
			default:
				Logger::error("Unknown ShaderDataType");
			}
		}
		m_VertexBuffers.push_back(vertexBuffer);
	}
#endif

	const std::vector<SP_VertexBuffer>& VertexArray::getVertexBuffers() const {
		return m_vertexBuffers;
	}

	SP_IndexBuffer VertexArray::getIndexBuffer() const
	{
		return m_indexBuffer;
	}

	void VertexArray::SetIndexBuffer(SP_IndexBuffer const& spIndexBuffer)
	{
		glBindVertexArray(m_rendererID);
		spIndexBuffer->Bind();
		//glVertexArrayElementBuffer(m_RendererID, indexBuffer->GetBufferID());	// In case we want to use DSA, we can remove glBindVertexArray and indexBufferBind and use this
		m_indexBuffer = spIndexBuffer;
	}

}
