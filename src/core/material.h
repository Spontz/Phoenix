// material.h
// Spontz Demogroup


#ifndef MATERIAL_H
#define MATERIAL_H


#include "main.h"

#include "core/demokernel.h"
#include "core/texture.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

// Specs: http://assimp.sourceforge.net/lib_html/materials.html
class Material {
public:
	string		name;
	vector<int> textures;
	glm::vec3	colDiffuse;
	glm::vec3	colSpecular;
	glm::vec3	colAmbient;
	bool		wireframe;	// The material is in wireframe?
	int			blendFunc;	// Blend function

	Material();
	void Load(const aiMaterial *pMaterial, string modelDirectory, string modelFilename);
	
private:
	vector<int> loadTextures(const aiMaterial *mat, aiTextureType type, string typeName);

	const aiMaterial	*m_pMaterial;
	string				m_ModelDirectory;	// Path of the model file
	string				m_ModelFilename;	// Name of the model file


};
#endif