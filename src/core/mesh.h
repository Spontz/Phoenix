// mesh.h
// Spontz Demogroup


#ifndef MESH_H
#define MESH_H


#include "main.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "core/demokernel.h"
#include "core/texture.h"
#include "core/shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
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
	GLint IDs[NUM_BONES_PER_VERTEX];
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
	glm::vec3		Position;	// 0 start
	glm::vec3		Normal;		// 3 * 4 = 12 start
	glm::vec2		TexCoords;	// 12+12 = 24start
	glm::vec3		Tangent;	// 24+ 8 = 32start
	glm::vec3		Bitangent;	// 32+12 = 44start
	VertexBoneData	Bone;		// BoneID: 44+12 = 56start
								// Weight: 56+16 = 72start
								// End: 72+16 = 88end
};



class Mesh {
public:
	vector<Vertex>			vertices;
	vector<unsigned int>	indices;
	vector<int>				textures;
	GLuint					VAO;
	

	Mesh(const aiScene *pScene, const aiMesh *pMesh, vector<Vertex> vertices, vector<unsigned int> indices, vector<int> textures);
	// render the mesh
	void Draw(Shader shader);
	
private:
	unsigned int VBO, EBO;
	const aiScene		*m_pScene;
	const aiMesh		*m_pMesh;

	// initializes all the buffer objects/arrays
	void setupMesh();
};
#endif