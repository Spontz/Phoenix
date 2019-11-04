// mesh.cpp
// Spontz Demogroup

#include "main.h"
#include "core/texture.h"
#include "core/shader.h"
#include "core/mesh.h"


#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#define POSITION_LOCATION		0
#define NORMAL_LOCATION			1
#define TEX_COORD_LOCATION		2
#define TANGENT_LOCATION		3
#define BITANGENT_LOCATION		4
#define BONE_ID_LOCATION		5
#define BONE_WEIGHT_LOCATION	6

using namespace std;

void VertexBoneData::AddBoneData(unsigned int BoneID, float Weight)
{
	for (unsigned int i = 0; i < NUM_BONES_PER_VERTEX; ++i) {
		if (Weights[i] == 0.0)
		{
			IDs[i] = BoneID;
			Weights[i] = Weight;
			return;
		}
	}

	// should never get here - more bones than we have space for
	assert(0);
}

Mesh::Mesh(const aiMesh *pMesh, vector<Vertex> vertices, vector<unsigned int> indices, const aiMaterial *pMaterial, string directory, string filename)
{
	this->m_pMesh = pMesh;
	this->vertices = vertices;
	this->indices = indices;

	// Setup the material of our mesh (each mesh has only one material)
	this->material.Load(pMaterial, directory, filename);

	// now that we have all the required data, set the vertex buffers and its attribute pointers.
	setupMesh();
}

// initializes all the buffer objects/arrays
void Mesh::setupMesh()
{
	// create VAO, VBO and ElementBuffer
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	// load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// A great thing about structs is that their memory layout is sequential for all its items.
	// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
	// again translates to 3/2 floats which translates to a byte array.
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	// set the vertex attribute pointers
	// vertex Positions
	glEnableVertexAttribArray(POSITION_LOCATION);
	glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	// vertex normals
	glEnableVertexAttribArray(NORMAL_LOCATION);
	glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	// vertex texture coords
	glEnableVertexAttribArray(TEX_COORD_LOCATION);
	glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	// vertex tangent
	glEnableVertexAttribArray(TANGENT_LOCATION);
	glVertexAttribPointer(TANGENT_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
	// vertex bitangent
	glEnableVertexAttribArray(BITANGENT_LOCATION);
	glVertexAttribPointer(BITANGENT_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

	// Bone Vertex ID's as Unsigned INT
	glEnableVertexAttribArray(BONE_ID_LOCATION);
	glVertexAttribIPointer(BONE_ID_LOCATION, 4, GL_UNSIGNED_INT, sizeof(Vertex), (void*)(offsetof(Vertex, Bone) + offsetof(VertexBoneData, IDs)));
	
	// Bone Vertex Weights
	glEnableVertexAttribArray(BONE_WEIGHT_LOCATION);
	glVertexAttribPointer(BONE_WEIGHT_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, Bone) + offsetof(VertexBoneData, Weights)));

	glBindVertexArray(0);
}

// render the mesh
void Mesh::Draw(GLuint shaderID)
{
	// Send material properties
	glUniform3fv(glGetUniformLocation(shaderID, "color_ambient"), 1, &material.colAmbient[0]);
	glUniform3fv(glGetUniformLocation(shaderID, "color_specular"), 1, &material.colSpecular[0]);
	glUniform1f(glGetUniformLocation(shaderID, "strenght_specular"), material.strenghtSpecular);
	glUniform3fv(glGetUniformLocation(shaderID, "color_diffuse"), 1, &material.colDiffuse[0]);

	// Send textures
	unsigned int numTextures = static_cast<unsigned int>(material.textures.size());
	for (unsigned int i = 0; i < numTextures; i++)
	{
		if (material.textures[i].ID == -1) // Avoid illegal access
			return;
		Texture *tex = DEMO->textureManager.texture[material.textures[i].ID];
		// active proper texture unit before binding
		tex->active(i); // TODO: In theory, this active is not needed, because is done during the "tex->bind(i)"
		glUniform1i(glGetUniformLocation(shaderID, material.textures[i].shaderName.c_str()), i);
		//glUniform1i(glGetUniformLocation(shaderID, tex->shaderName.c_str()), i);
		// and finally bind the texture
		tex->bind(i);
	}

	// draw mesh
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, (int)indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	// always good practice to set everything back to defaults once configured.
	// TODO: Should we reset all the Tex units, right? Not only the first one!
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
}




