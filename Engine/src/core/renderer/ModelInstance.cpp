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
		m_pPrevModelMatrix(nullptr),
		m_vBufferMM_ID(0)
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
			// Search if the Vertex Buffer is already present
			int32_t vbID = spMesh->m_VertexArray->getVertexBufferIDByName("aInstanceMatrix"); 
			
			// If is not present, we create the new VB and add it
			if (vbID == -1) {
				auto spVB = std::make_shared<VertexBuffer>(m_amount * sizeof(glm::mat4));
				spVB->SetLayout({ {ShaderDataType::Mat4, "aInstanceMatrix"} });

				spMesh->m_VertexArray->AddVertexBuffer(spVB);
				m_vBufferMM_ID = spMesh->m_VertexArray->getVertexBufferIDByName("aInstanceMatrix");
			}
			else {
				m_vBufferMM_ID = vbID; // If we already know the buffer ID
			}

			// Prevent index to get out of bounds
			if (m_vBufferMM_ID > (spMesh->m_VertexArray->getVertexBufferSize() - 1)) {
				Logger::error("ModelInstance constructor error, detected m_vBufferMM_ID greater than VertexBuffer size, instance model will not display correctly!");
				m_vBufferMM_ID = 0;
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
			VBs[m_vBufferMM_ID]->SetData(&m_pModelMatrix[0], m_amount * sizeof(glm::mat4));
			VBs[m_vBufferMM_ID]->Unbind();
		}
	}

}
