#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "core/demokernel.h"
#include "core/mesh.h"
#include "core/shader.h"
#include "main.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

class Model
{
public:
	vector<int> textures_loaded;
	vector<Mesh> meshes;
	string directory;
	string filename;
	bool gammaCorrection;

	// constructor, expects a filepath to a 3D model.
	Model::Model(string const &path, bool gamma = false) : gammaCorrection(gamma)
	{
		loadModel(path);
	}
	// draws the model, and thus all its meshes
	void Draw(Shader shader);

private:
	// Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
	void loadModel(string const &path);
	// Processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	void processNode(aiNode *node, const aiScene *scene);
	Mesh processMesh(aiMesh *mesh, const aiScene *scene);

	// Checks all material textures of a given type and loads the textures if they're not loaded yet.
	// the required info is returned as a Texture array (from texture manager).
	vector<int> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName);
};


#endif