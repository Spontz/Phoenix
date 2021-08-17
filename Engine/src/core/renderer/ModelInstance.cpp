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

	ModelInstance::ModelInstance(SP_Model model, unsigned int amount)
		:
		pModel(model),
		amount(amount),
		pModelMatrix(nullptr),
		pPrevModelMatrix(nullptr),
		matricesBuffer(0)
	{
		if (!model)
			return;

		// Init model matrices
		if (pModelMatrix)
			delete[] pModelMatrix;

		if (pPrevModelMatrix)
			delete[] pPrevModelMatrix;

		pModelMatrix = new glm::mat4[amount];
		pPrevModelMatrix = new glm::mat4[amount];
		for (unsigned int i = 0; i < amount; i++)
		{
			pModelMatrix[i] = glm::mat4(1.0f);
			pPrevModelMatrix[i] = glm::mat4(1.0f);
		}


		// Create the new Vertex Buffer to apply to the meshes
		auto pVB = std::make_shared<VertexBuffer>(static_cast<uint32_t>(static_cast<size_t>(amount) * sizeof(glm::mat4)));
		pVB->SetLayout({
			{ ShaderDataType::Mat4,	"aInstancePos"}
			});

		// Add the new vertex buffer to mesh vertex array
		for (unsigned int i = 0; i < pModel->meshes.size(); ++i)
			pModel->meshes[i]->m_VertexArray->AddVertexBuffer(pVB);
	}

	ModelInstance::~ModelInstance()
	{
		if (pModelMatrix)
			delete[] pModelMatrix;
		if (pPrevModelMatrix)
			delete[] pPrevModelMatrix;

	}

	void ModelInstance::drawInstanced(GLuint shaderID, uint32_t startTexUnit)
	{
		for (const auto& mesh : pModel->meshes)
		{
			mesh->setMaterialShaderVars(shaderID, startTexUnit);

			// draw mesh
			mesh->m_VertexArray->bind();
			glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(mesh->m_indices.size()), GL_UNSIGNED_INT, 0, static_cast<GLsizei>(amount));
			
			mesh->m_VertexArray->unbind();
		}
	}

	void ModelInstance::copyMatrices(int instance)
	{
		pPrevModelMatrix[instance] = pModelMatrix[instance];
	}

	void ModelInstance::updateMatrices()
	{
		for (const auto& mesh : pModel->meshes)
		{
			// Update matrices buffers to GPU
			const auto& VBs = mesh->m_VertexArray->getVertexBuffers();
			VBs[1]->SetData(&pModelMatrix[0], amount * sizeof(glm::mat4)); // Be careful with "vb[1]"!! -> TODO: Get rid of this hardcode!!
			VBs[1]->Unbind();
		}
	}

}
