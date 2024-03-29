// Buffer.h
// Spontz Demogroup

#pragma once

#include "core/utils/LoggerDeclarations.h"
#include "SpzDeclare.h"

#include <array>

namespace Phoenix {

	enum class ShaderDataType : uint32_t
	{
		None = 0,
		Float = 1,
		Float2 = 2,
		Float3 = 3,
		Float4 = 4,
		Mat3 = 5,
		Mat4 = 6,
		Int = 7,
		Int2 = 8,
		Int3 = 9,
		Int4 = 10,
		UInt = 11,
		UInt2 = 12,
		UInt3 = 13,
		UInt4 = 14,
		Bool = 15
	};

	struct ShaderDataTypeTable final {
		ShaderDataType	type = ShaderDataType::None;
		GLenum			GLBaseType = GL_NONE;
		uint32_t		elementCount = 0;
		uint32_t		sizeInBytes = 0;
	};

	constexpr std::array<ShaderDataTypeTable, 16> ShaderDataTypes{ {
			// ShaderDataType			GLBaseType			elementCount	sizeInBytes
			{ ShaderDataType::None,		GL_NONE,			0,				0,			},
			{ ShaderDataType::Float,	GL_FLOAT,			1,				4			},
			{ ShaderDataType::Float2,	GL_FLOAT,			2,				4 * 2		},
			{ ShaderDataType::Float3,	GL_FLOAT,			3,				4 * 3		},
			{ ShaderDataType::Float4,	GL_FLOAT,			4,				4 * 4		},
			{ ShaderDataType::Mat3,		GL_FLOAT,			3,				4 * 3 * 3	},
			{ ShaderDataType::Mat4,		GL_FLOAT,			4,				4 * 4 * 4	},
			{ ShaderDataType::Int,		GL_INT,				1,				4			},
			{ ShaderDataType::Int2,		GL_INT,				2,				4 * 2		},
			{ ShaderDataType::Int3,		GL_INT,				3,				4 * 3		},
			{ ShaderDataType::Int4,		GL_INT,				4,				4 * 4		},
			{ ShaderDataType::UInt,		GL_UNSIGNED_INT,	1,				4			},
			{ ShaderDataType::UInt2,	GL_UNSIGNED_INT,	2,				4 * 2		},
			{ ShaderDataType::UInt3,	GL_UNSIGNED_INT,	3,				4 * 3		},
			{ ShaderDataType::UInt4,	GL_UNSIGNED_INT,	4,				4 * 4		},
			{ ShaderDataType::Bool,		GL_BOOL,			1,				4			},
	} };

	class BufferElement final
	{
	public:
		BufferElement() = default;
		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
			:
			Name(name),
			Type(type),
			Offset(0),
			Normalized(normalized),
			DataType(getShaderDataType(type))
		{
		}

	public:
		static const ShaderDataTypeTable& getShaderDataType(ShaderDataType type)
		{
			const auto tmp = static_cast<size_t>(type);
			if (tmp >= ShaderDataTypes.size()) {
				Logger::error("Unknown ShaderDataType");
				static ShaderDataTypeTable r;
				return r;
			}
			return ShaderDataTypes[tmp];
		}

	public:
		std::string Name;
		ShaderDataType Type = ShaderDataType::None;
		size_t Offset = 0;
		bool Normalized = false;
		ShaderDataTypeTable DataType;
	};

	class BufferLayout final
	{
	public:
		BufferLayout() = default;

		BufferLayout(std::initializer_list<BufferElement> elements)
			: m_Elements(elements)
		{
			CalculateOffsetsAndStride();
		}

		uint32_t GetStride() const { return m_Stride; }
		const std::vector<BufferElement>& GetElements() const { return m_Elements; }

		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }

	private:
		void CalculateOffsetsAndStride()
		{
			size_t offset = 0;
			m_Stride = 0;
			for (auto& element : m_Elements) {
				element.Offset = offset;
				offset += size_t(element.DataType.sizeInBytes);
				m_Stride += element.DataType.sizeInBytes;
			}
		}

	private:
		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride = 0;
	};


	class VertexBuffer final
	{
	public:
		VertexBuffer(size_t size);
		VertexBuffer(const void* data, size_t size);
		~VertexBuffer();

	public:
		void Bind() const;
		void Unbind() const;
		void SetData(const void* data, size_t size);
		uint32_t GetBufferID() const { return m_RendererID; };
		const BufferLayout& GetLayout() const { return m_Layout; }
		void SetLayout(const BufferLayout& layout) { m_Layout = layout; }

	private:
		uint32_t m_RendererID;
		BufferLayout m_Layout;
	};

	// TODO: CreateIndexBuffer16 and IndexBuffer32
	class IndexBuffer final
	{
	public:
		IndexBuffer(const uint32_t* pIndices, uint32_t count);
		~IndexBuffer();

	public:
		void Bind() const;
		void Unbind() const;
		uint32_t GetBufferID() const { return m_RendererID; };
		uint32_t GetCount() const { return m_Count; }

	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
	};

	SPZ_DECLARE_SMART_POINTERS(VertexBuffer);
	SPZ_DECLARE_SMART_POINTERS(IndexBuffer);

}
