// VertexArray.h
// Spontz Demogroup

#pragma once

#include "core/renderer/Buffer.h"

namespace Phoenix {

	class VertexArray;
	using SP_VertexArray = std::shared_ptr<VertexArray>;
	using WP_VertexArray = std::weak_ptr<VertexArray>;

	class VertexArray final
	{
	public:
		VertexArray();
		~VertexArray();

	public:
		void Bind() const;
		void Unbind() const;

		void AddVertexBuffer(SP_VertexBuffer spVertexBuffer);
		void SetIndexBuffer(SP_IndexBuffer spIndexBuffer);

		const std::vector<SP_VertexBuffer>& GetVertexBuffers() const;
		SP_IndexBuffer GetIndexBuffer() const;

	private:
		uint32_t m_RendererID; // Our "VAO"
		uint32_t m_VertexBufferIndex = 0;
		std::vector<SP_VertexBuffer> m_VertexBuffers;
		SP_IndexBuffer m_IndexBuffer;
	};
}