// modelinstance.cpp
// Spontz Demogroup

#include "main.h"
#include "core/texture.h"
#include "core/shader.h"
#include "core/mesh.h"
#include "core/model.h"
#include "core/modelinstance.h"

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
    modelMatrices(nullptr),
    matricesBuffer(0)
{
	if (!model)
		return;
	
    // Init model matrices
    if (modelMatrices)
		delete[] modelMatrices;

	modelMatrices = new glm::mat4[amount];
    for (unsigned int i = 0; i < amount; i++)
    {
        modelMatrices[i] = glm::mat4(1.0f);
    }


    // configure instanced array
    // -------------------------
    glGenBuffers(1, &matricesBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, matricesBuffer);
    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_DYNAMIC_DRAW);// GL_STATIC_DRAW);

    // set transformation matrices as an instance vertex attribute (with divisor 1)
    // note: we're cheating a little by taking the, now publicly declared, VAO of the model's mesh(es) and adding new vertexAttribPointers
    // normally you'd want to do this in a more organized fashion, but for learning purposes this will do.
    // -----------------------------------------------------------------------------------------------------------------------------------
    for (unsigned int i = 0; i < model->meshes.size(); i++)
    {
        glBindVertexArray(model->meshes[i].m_VAO);
        // set attribute pointers for matrix (4 times vec4)
        glEnableVertexAttribArray(INSTANCED_MAT4_LOCATION_1);
        glVertexAttribPointer(INSTANCED_MAT4_LOCATION_1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
        glEnableVertexAttribArray(INSTANCED_MAT4_LOCATION_2);
        glVertexAttribPointer(INSTANCED_MAT4_LOCATION_2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
        glEnableVertexAttribArray(INSTANCED_MAT4_LOCATION_3);
        glVertexAttribPointer(INSTANCED_MAT4_LOCATION_3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
        glEnableVertexAttribArray(INSTANCED_MAT4_LOCATION_4);
        glVertexAttribPointer(INSTANCED_MAT4_LOCATION_4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

        glVertexAttribDivisor(INSTANCED_MAT4_LOCATION_1, 1);
        glVertexAttribDivisor(INSTANCED_MAT4_LOCATION_2, 1);
        glVertexAttribDivisor(INSTANCED_MAT4_LOCATION_3, 1);
        glVertexAttribDivisor(INSTANCED_MAT4_LOCATION_4, 1);

        glBindVertexArray(0);
    }

}

ModelInstance::~ModelInstance()
{
}

void ModelInstance::DrawInstanced(GLuint shaderID)
{
    for (unsigned int i = 0; i < model->meshes.size(); i++)
    {
        Mesh* my_mesh = &(model->meshes[i]);

        my_mesh->DrawMaterials(shaderID);

        // Update matrices buffers to GPU
        glBindBuffer(GL_ARRAY_BUFFER, matricesBuffer);
        glBufferData(GL_ARRAY_BUFFER, amount*sizeof(glm::mat4), &modelMatrices[0], GL_DYNAMIC_DRAW);

        // draw mesh
        glBindVertexArray(model->meshes[i].m_VAO);
        glDrawElementsInstanced(GL_TRIANGLES, model->meshes[i].m_indices.size(), GL_UNSIGNED_INT, 0, amount);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // always good practice to set everything back to defaults once configured.
        //glBindTextureUnit(0, 0); --> TODO: This gives error on some graphics card (https://community.intel.com/t5/Graphics/intel-uhd-graphics-630-with-latest-driver-will-cause-error-when/td-p/1161376)
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        //glBindVertexArray(model->meshes[i].m_VAO);
        //glDrawElementsInstanced(GL_TRIANGLES, model->meshes[i].m_indices.size(), GL_UNSIGNED_INT, 0, amount);
        //glBindVertexArray(0);
    }
}

