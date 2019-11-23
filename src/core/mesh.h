// mesh.h
// Spontz Demogroup


#ifndef MESH_H
#define MESH_H


#include "main.h"

#include <glm/glm.hpp>

#include "core/demokernel.h"
#include "core/material.h"
#include "core/texture.h"
#include "core/shader.h"

#include <string>
#include <vector>

using namespace std;
#define NUM_BONES_PER_VERTEX	4 // Number of Bones per Vertex

struct BoneInfo
{
	glm::mat4 BoneOffset;
	glm::mat4 FinalTransformation;

	BoneInfo()
	{
		BoneOffset = glm::mat4(0.0f);
		FinalTransformation = glm::mat4(0.0f);
	}
};

struct VertexBoneData
{
	GLuint IDs[NUM_BONES_PER_VERTEX];
	GLfloat Weights[NUM_BONES_PER_VERTEX];

	VertexBoneData() { Reset(); }
	
	void Reset()
	{
		for (unsigned int i = 0; i < NUM_BONES_PER_VERTEX; ++i)
		{
			IDs[i] = 0;
			Weights[i] = 0;
		}
	}

	void AddBoneData(unsigned int BoneID, float Weight);
};

struct Vertex {
	glm::vec3		Position;
	glm::vec3		Normal;
	glm::vec2		TexCoords;
	glm::vec3		Tangent;
	glm::vec3		Bitangent;
	VertexBoneData	Bone;
};

class Mesh {
public:
	vector<Vertex>			vertices;
	vector<unsigned int>	indices;
	Material				material;
	GLuint					VAO;
	string					nodeName;
	glm::mat4				meshTransform; // Transformation Matrix for positioning the mesh
	

	Mesh(string nodeName, const aiMesh *pMesh, vector<Vertex> vertices, vector<unsigned int> indices, const aiMaterial *pMaterial, string directory, string filename);
	// render the mesh
	void Draw(GLuint shaderID);
	
private:
	unsigned int		VBO, EBO;
	const aiMesh		*m_pMesh;

	// initializes all the buffer objects/arrays
	void setupMesh();
};
#endif