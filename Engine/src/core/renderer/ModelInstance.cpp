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

	ModelInstance::ModelInstance(SP_Model const& spModel, uint32_t amount)
		:
		m_pModel(spModel),
		m_amount(amount),
		m_pModelMatrix(nullptr),
		m_pPrevModelMatrix(nullptr),
		m_matricesBuffer(0)
	{
		if (!spModel)
			return;

		// Init model matrices
		if (m_pModelMatrix)
			delete[] m_pModelMatrix;

		if (m_pPrevModelMatrix)
			delete[] m_pPrevModelMatrix;

		m_pModelMatrix = new glm::mat4[m_amount];
		m_pPrevModelMatrix = new glm::mat4[m_amount];
		for (unsigned int i = 0; i < m_amount; i++) {
			m_pModelMatrix[i] = glm::mat4(1.0f);
			m_pPrevModelMatrix[i] = glm::mat4(1.0f);
		}


		// Create the new Vertex Buffer to apply to the meshes
		auto pVB = std::make_shared<VertexBuffer>(m_amount * sizeof(glm::mat4));
		pVB->SetLayout({{ShaderDataType::Mat4, "aInstancePos"}});

		// Add the new vertex buffer to mesh vertex array
		for (auto const& spMesh : m_pModel->meshes)
			spMesh->m_VertexArray->AddVertexBuffer(pVB);
	}

	ModelInstance::~ModelInstance()
	{
		if (m_pModelMatrix)
			delete[] m_pModelMatrix;
		if (m_pPrevModelMatrix)
			delete[] m_pPrevModelMatrix;
		
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
