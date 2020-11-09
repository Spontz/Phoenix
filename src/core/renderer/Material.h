// Material.h
// Spontz Demogroup


#pragma once
#include "main.h"
#include "core/Demokernel.h"
#include "core/renderer/Texture.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>


struct textureStack
{
	Texture*	tex;			// Texture
	std::string	shaderName;		// Name that receives the texture in the shader
	int			blendOperation;	// Blend operation with the base color
	float		strength;		// Strength factor

	textureStack()
	{ 
		tex = nullptr;
		shaderName = "";
		blendOperation = 0;
		strength = 0;
	}
};

// Specs: http://assimp.sourceforge.net/lib_html/materials.html
class Material {
public:
	std::string					name;
	std::vector<textureStack>	textures;
	glm::vec3					colDiffuse;
	glm::vec3					colSpecular;
	glm::vec3					colAmbient;
	float						strenghtSpecular;

	Material();
	void Load(const aiMaterial *pMaterial, std::string modelDirectory, std::string modelFilename);
	
private:
	std::vector<textureStack> loadTextures(const aiMaterial *mat, aiTextureType type, std::string typeName);

	const aiMaterial	*m_pMaterial;
	std::string			m_ModelDirectory;	// Path of the model file
	std::string			m_ModelFilename;	// Name of the model file
};
