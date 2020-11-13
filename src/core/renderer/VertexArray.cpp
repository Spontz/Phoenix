// VertexArray.cpp
// Spontz Demogroup

#include "main.h"
#include "VertexArray.h"

static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
{
	switch (type)
	{
	case ShaderDataType::Float:    return GL_FLOAT;
	case ShaderDataType::Float2:   return GL_FLOAT;
	case ShaderDataType::Float3:   return GL_FLOAT;
	case ShaderDataType::Float4:   return GL_FLOAT;
	case ShaderDataType::Mat3:     return GL_FLOAT;
	case ShaderDataType::Mat4:     return GL_FLOAT;
	case ShaderDataType::Int:      return GL_INT;
	case ShaderDataType::Int2:     return GL_INT;
	case ShaderDataType::Int3:     return GL_INT;
	case ShaderDataType::Int4:     return GL_INT;
	case ShaderDataType::UInt:     return GL_UNSIGNED_INT;
	case ShaderDataType::UInt2:    return GL_UNSIGNED_INT;
	case ShaderDataType::UInt3:    return GL_UNSIGNED_INT;
	case ShaderDataType::UInt4:    return GL_UNSIGNED_INT;
	case ShaderDataType::Bool:     return GL_BOOL;
	}

	Logger::error("Unknown ShaderDataType");
	return 0;
}

VertexArray::VertexArray()
	:
	m_IndexBuffer(nullptr)
{
	glCreateVertexArrays(1, &m_RendererID);
}

VertexArray::~VertexArray()
{
	if (m_RendererID!=0)
		glDeleteVertexArrays(1, &m_RendererID);
}

void VertexArray::Bind() const
{
	glBindVertexArray(m_RendererID);
}

void VertexArray::Unbind() const
{
	glBindVertexArray(0);
}

void VertexArray::AddVertexBuffer(VertexBuffer* vertexBuffer)
{
	glBindVertexArray(m_RendererID);
	vertexBuffer->Bind();
	const auto& layout = vertexBuffer->GetLayout();

	for (const auto& element : layout)
	{
		switch (element.Type)
		{
		case ShaderDataType::Float:
		case ShaderDataType::Float2:
		case ShaderDataType::Float3:
		case ShaderDataType::Float4:
		{
			glEnableVertexAttribArray(m_VertexBufferIndex);
			glVertexAttribPointer(m_VertexBufferIndex,
				element.GetComponentCount(),
				ShaderDataTypeToOpenGLBaseType(element.Type),
				element.Normalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				(const void*)element.Offset);
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
			glEnableVertexAttribArray(m_VertexBufferIndex);
			glVertexAttribIPointer(m_VertexBufferIndex,
				element.GetComponentCount(),
				ShaderDataTypeToOpenGLBaseType(element.Type),
				layout.GetStride(),
				(const void*)element.Offset);
			m_VertexBufferIndex++;
			break;
		}
		case ShaderDataType::Mat3:
		case ShaderDataType::Mat4:
		{
			uint8_t count = element.GetComponentCount();
			for (uint8_t i = 0; i < count; i++)
			{
				glEnableVertexAttribArray(m_VertexBufferIndex);
				glVertexAttribPointer(m_VertexBufferIndex,
					count,
					ShaderDataTypeToOpenGLBaseType(element.Type),
					element.Normalized ? GL_TRUE : GL_FALSE,
					layout.GetStride(),
					(const void*)(element.Offset + sizeof(float) * count * i));
				glVertexAttribDivisor(m_VertexBufferIndex, 1);
				m_VertexBufferIndex++;
			}
			break;
		}
		default:
			Logger::error("Unknown ShaderDataType");
		}
	}

	m_VertexBuffers.push_back(vertexBuffer);
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
				element.GetComponentCount(),
				ShaderDataTypeToOpenGLBaseType(element.Type),
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
				element.GetComponentCount(),
				ShaderDataTypeToOpenGLBaseType(element.Type),
				element.Offset);

			glVertexArrayAttribBinding(m_RendererID, m_VertexBufferIndex, 0);	// remove Binding hardcoded to 0

			m_VertexBufferIndex++;
			break;
		}
		case ShaderDataType::Mat3:
		case ShaderDataType::Mat4:
		{
			uint8_t count = element.GetComponentCount();
			for (uint8_t i = 0; i < count; i++)
			{
				glEnableVertexArrayAttrib(m_RendererID, m_VertexBufferIndex);
				glVertexArrayAttribFormat(m_RendererID, m_VertexBufferIndex,
					element.GetComponentCount(),
					ShaderDataTypeToOpenGLBaseType(element.Type),
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

void VertexArray::SetIndexBuffer(IndexBuffer* indexBuffer)
{
	glBindVertexArray(m_RendererID);
	indexBuffer->Bind();
	//glVertexArrayElementBuffer(m_RendererID, indexBuffer->GetBufferID());	// In case we want to use DSA, we can remove glBindVertexArray and indexBufferBind and use this
	m_IndexBuffer = indexBuffer;
}
