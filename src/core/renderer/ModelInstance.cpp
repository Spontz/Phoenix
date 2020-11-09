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

#define INSTANCED_MAT4_LOCATION_1	7
#define INSTANCED_MAT4_LOCATION_2	8
#define INSTANCED_MAT4_LOCATION_3	9
#define INSTANCED_MAT4_LOCATION_4	10

ModelInstance::ModelInstance(Model* model, unsigned int amount)
    :
    model(model),
    amount(amount),
    modelMatrix(nullptr),
    prev_modelMatrix(nullptr),
    matricesBuffer(0)
{
	if (!model)
		return;
	
    // Init model matrices
    if (modelMatrix)
		delete[] modelMatrix;

    if (prev_modelMatrix)
        delete[] prev_modelMatrix;

	modelMatrix = new glm::mat4[amount];
    prev_modelMatrix = new glm::mat4[amount];
    for (unsigned int i = 0; i < amount; i++)
    {
        modelMatrix[i] = glm::mat4(1.0f);
        prev_modelMatrix[i] = glm::mat4(1.0f);
    }


    // Create the new Vertex Buffer to apply to the meshes
    
    VertexBuffer* vertexBuffer = new VertexBuffer(amount * sizeof(glm::mat4));
    vertexBuffer->SetLayout({
        { ShaderDataType::Mat4,	"aInstancePos"}
        });

    for (unsigned int i = 0; i < model->meshes.size(); i++)
    {
        model->meshes[i].m_VertexArray->AddVertexBuffer(vertexBuffer);
    }
}

ModelInstance::~ModelInstance()
{
    if (modelMatrix)
        delete[] modelMatrix;
    if (prev_modelMatrix)
        delete[] prev_modelMatrix;

}

void ModelInstance::DrawInstanced(GLuint shaderID)
{
    for (unsigned int i = 0; i < model->meshes.size(); i++)
    {
        Mesh* my_mesh = &(model->meshes[i]);

        my_mesh->DrawMaterials(shaderID);

        // Update matrices buffers to GPU
        std::vector<VertexBuffer*> vb = my_mesh->m_VertexArray->GetVertexBuffers();
        vb[1]->SetData(&modelMatrix[0], amount * sizeof(glm::mat4)); // TODO: Get rid of this hardcode

        // draw mesh
        model->meshes[i].m_VertexArray->Bind();
        glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(model->meshes[i].m_indices.size()), GL_UNSIGNED_INT, 0, static_cast<GLsizei>(amount));
        vb[1]->Unbind();
        model->meshes[i].m_VertexArray->Unbind();
    }
    
    // always good practice to set everything back to defaults once configured.
    //glBindTextureUnit(0, 0); --> TODO: This gives error on some graphics card (https://community.intel.com/t5/Graphics/intel-uhd-graphics-630-with-latest-driver-will-cause-error-when/td-p/1161376)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void ModelInstance::copyMatrices(int instance)
{
    prev_modelMatrix[instance] = modelMatrix[instance];
}

