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

#define NUM_BONES_PER_VERTEX	4 // Number of bones information to store per vertex

struct VertexBoneData
{
	unsigned int IDs[NUM_BONES_PER_VERTEX];
	float Weights[NUM_BONES_PER_VERTEX];

	VertexBoneData()
	{
		Reset();
	}

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
	VertexBoneData	BoneData;
};

class Mesh {
public:
	vector<Vertex>			vertices;
	vector<unsigned int>	indices;
	vector<int>				textures;
	GLuint					VAO;

	// TODO: implement baseVertex and baseIndex, for drawing meshes even more faster with one single array of vertices
	// add: unsigned int baseVertex, unsigned int baseIndex
	Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<int> textures);
	// render the mesh
	void Draw(Shader shader);

private:
	unsigned int VBO, EBO;

	// initializes all the buffer objects/arrays
	void setupMesh();
};
#endif