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

	ModelInstance::ModelInstance(Model* model, unsigned int amount)
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
		VertexBuffer* vertexBuffer = new VertexBuffer(amount * sizeof(glm::mat4));
		vertexBuffer->SetLayout({
			{ ShaderDataType::Mat4,	"aInstancePos"}
			});

		// Add the new vertex buffer to mesh vertex array
		for (unsigned int i = 0; i < pModel->meshes.size(); i++)
		{
			pModel->meshes[i].m_VertexArray->AddVertexBuffer(vertexBuffer);
		}
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
		for (auto& mesh : pModel->meshes)
		{
			mesh.setMaterialShaderVars(shaderID, startTexUnit);

			// draw mesh
			mesh.m_VertexArray->Bind();
			glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(mesh.m_indices.size()), GL_UNSIGNED_INT, 0, static_cast<GLsizei>(amount));
			
			mesh.m_VertexArray->Unbind();
		}
	}

	void ModelInstance::copyMatrices(int instance)
	{
		pPrevModelMatrix[instance] = pModelMatrix[instance];
	}

	void ModelInstance::updateMatrices()
	{
		for (auto& mesh : pModel->meshes)
		{
			// Update matrices buffers to GPU
			std::vector<VertexBuffer*> vb = mesh.m_VertexArray->GetVertexBuffers();
			vb[1]->SetData(&pModelMatrix[0], amount * sizeof(glm::mat4)); // Be careful with "vb[1]"!! -> TODO: Get rid of this hardcode!!
			vb[1]->Unbind();
		}
	}

}