// ModelInstance.cpp
// Spontz Demogroup

#include "main.h"
#include "core/renderer/Texture.h"
#include "core/renderer/Shader.h"
#include "core/renderer/Mesh.h"
#include "core/renderer/Model.h"
#include "core/renderer/ModelInstance.h"
#include "core/renderer/VertexArray.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

namespace Phoenix {

	ModelInstance::ModelInstance(SP_Model spModel, uint32_t amount)
		:
		m_pModel(spModel),
		m_amount(amount),
		m_pModelMatrix(nullptr),
		m_pPrevModelMatrix(nullptr)
	{
		if (amount == 0)
			return;
		m_pModelMatrix = std::make_unique<glm::mat4[]>(m_amount);
		m_pPrevModelMatrix = std::make_unique<glm::mat4[]>(m_amount);
		for (uint32_t i = 0; i < m_amount; i++) {
			m_pModelMatrix[i] = glm::mat4(1.0f);
			m_pPrevModelMatrix[i] = glm::mat4(1.0f);
		}

		// Check if each of the meshes of the Model has already the Vertex buffer, in case is already there, skip the new VB creation
		for (auto const& spMesh : m_pModel->meshes) {
			bool vertexBufferFound = false;
			// Search if the matrix is already in the Vertex Buffer
			const auto& VBs = spMesh->m_VertexArray->getVertexBuffers();
						
			for (int32_t i = 0; i < VBs.size(); i++) {
				const auto& BLayout = VBs[i]->GetLayout();
				const auto& elements = BLayout.GetElements();
				for (auto const& element : elements) {
					if (element.Name == "aInstancePos")
						vertexBufferFound = true;
				}
			}
			
			// If it's not found, add the vertex buffer
			if (vertexBufferFound == false) {
				auto spVB = std::make_shared<VertexBuffer>(m_amount * sizeof(glm::mat4));
				spVB->SetLayout({ {ShaderDataType::Mat4, "aInstancePos"} });

				spMesh->m_VertexArray->AddVertexBuffer(spVB);
			}
		}

	}

	ModelInstance::~ModelInstance()
	{
		m_amount = 0;
	}

	void ModelInstance::drawInstanced(float currentTime, GLuint shaderID, uint32_t startTexUnit)
	{
		if (m_pModel->playAnimation)
			m_pModel->setBoneTransformations(shaderID, currentTime);

		for (const auto& spMesh : m_pModel->meshes) {
			spMesh->setMaterialShaderVars(shaderID, startTexUnit);

			// draw mesh
			spMesh->m_VertexArray->bind();
			glDrawElementsInstanced(
				GL_TRIANGLES,
				static_cast<GLsizei>(spMesh->m_indices.size()),
				GL_UNSIGNED_INT,
				0,
				static_cast<GLsizei>(m_amount)
			);

			spMesh->m_VertexArray->unbind();
		}
	}

	void ModelInstance::copyMatrices(int instance)
	{
		m_pPrevModelMatrix[instance] = m_pModelMatrix[instance];
	}

	void ModelInstance::updateMatrices()
	{
		for (const auto& spMesh : m_pModel->meshes)
		{
			// Update matrices buffers to GPU
			const auto& VBs = spMesh->m_VertexArray->getVertexBuffers();
			VBs[1]->SetData(&m_pModelMatrix[0], m_amount * sizeof(glm::mat4)); // Be careful with "vb[1]"!! -> TODO: Get rid of this hardcode!!
			VBs[1]->Unbind();
		}
	}

}
