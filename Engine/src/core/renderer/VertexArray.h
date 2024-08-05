// VertexArray.h
// Spontz Demogroup

#pragma once

#include "core/renderer/Buffer.h"

namespace Phoenix {

	class VertexArray final
	{
	public:
		VertexArray();
		~VertexArray();

	public:
		void bind() const;
		void unbind() const;

		void AddVertexBuffer(SP_VertexBuffer const& spVertexBuffer);
		void SetIndexBuffer(SP_IndexBuffer const& spIndexBuffer);
		
		const std::vector<SP_VertexBuffer>& getVertexBuffers() const;
		const uint32_t getVertexBufferSize() const;
		const int32_t getVertexBufferIDByName(std::string_view vbName) const;
		SP_IndexBuffer getIndexBuffer() const;

	private:
		uint32_t m_rendererID; // Our "VAO"
		uint32_t m_vertexBufferIndex = 0;
		std::vector<SP_VertexBuffer> m_vertexBuffers;
		SP_IndexBuffer m_indexBuffer;
	};

	SPZ_DECLARE_SMART_POINTERS(VertexArray);
}
