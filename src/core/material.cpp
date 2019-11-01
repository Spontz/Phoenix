﻿// material.cpp
// Spontz Demogroup

#include "main.h"
#include "core/texture.h"
#include "core/material.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

Material::Material()
{
	this->name = "";
	this->colDiffuse = glm::vec3(0);
	this->colSpecular = glm::vec3(0);
	this->colAmbient = glm::vec3(0);
	this->strenghtSpecular = 1.0;
}

void Material::Load(const aiMaterial *pMaterial, string modelDirectory, string modelFilename)
{
	this->m_pMaterial = pMaterial;
	this->m_ModelDirectory = modelDirectory;
	this->m_ModelFilename = modelFilename;
	
	// we assume a convention for sampler names in the shaders. Each diffuse texture should be named
	// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
	// Same applies to other texture as the following list summarizes:
	// diffuse: texture_diffuseN
	// specular: texture_specularN
	// normal: texture_normalN
	// 1. diffuse maps
	vector<textureStack> diffuseMaps = loadTextures(pMaterial, aiTextureType_DIFFUSE, "texture_diffuse");
	textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
	LOG->Info(LOG_LOW, "  The mesh has %d diffuseMaps", diffuseMaps.size());
	// 2. specular maps
	vector<textureStack> specularMaps = loadTextures(pMaterial, aiTextureType_SPECULAR, "texture_specular");
	textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	LOG->Info(LOG_LOW, "  The mesh has %d specularMaps", specularMaps.size());
	// 3. normal maps
	std::vector<textureStack> normalMaps = loadTextures(pMaterial, aiTextureType_HEIGHT, "texture_normal");
	textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
	LOG->Info(LOG_LOW, "  The mesh has %d normalMaps", normalMaps.size());
	// 4. height maps
	std::vector<textureStack> heightMaps = loadTextures(pMaterial, aiTextureType_AMBIENT, "texture_height");
	textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
	LOG->Info(LOG_LOW, "  The mesh has %d heightMaps", heightMaps.size());
	// Unknown
	std::vector<textureStack> unknownMaps = loadTextures(pMaterial, aiTextureType_NONE, "texture_unknown");
	textures.insert(textures.end(), unknownMaps.begin(), unknownMaps.end());
	LOG->Info(LOG_LOW, "  The mesh has %d unknownMaps", unknownMaps.size());


	aiColor3D color;
	// Diffuse color
	pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);
	this->colDiffuse = glm::vec3(color.r, color.g, color.b);
	
	// Ambient color
	pMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color);
	this->colAmbient = glm::vec3(color.r, color.g, color.b);

	// Specular: Specular color = COLOR_SPECULAR * SHININESS_STRENGTH
	pMaterial->Get(AI_MATKEY_SHININESS_STRENGTH, this->strenghtSpecular);
	pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color);
	this->colSpecular = glm::vec3(color.r, color.g, color.b);
}

// checks all material textures of a given type and loads the textures if they're not loaded yet.
// the required info is returned as a Texture struct.
vector<textureStack> Material::loadTextures(const aiMaterial * mat, aiTextureType type, string typeName)
{
	vector<textureStack> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString filepath;
		string fullpath;
		ai_real blendFactor;
		aiTextureOp operation;
		mat->GetTexture(type, i, &filepath, NULL, NULL, &blendFactor, &operation);
		if (0 == strcmp(filepath.C_Str(), "$texture_dummy.bmp"))				// Prevent a bug in assimp: In some cases, the texture by default is named "$texture_dummy.bmp"
			filepath = m_ModelFilename.substr(0, m_ModelFilename.find_last_of('.')) + ".jpg";	// In that case, we change this to "<model_name.jpg>"
		fullpath = m_ModelDirectory + "/" + filepath.C_Str();
		textureStack tex;
		tex.ID	= DEMO->textureManager.addTexture(fullpath.c_str(), false, typeName);
		tex.blendOperation = operation;
		tex.strength = blendFactor;
		if (tex.ID > 0) {
			Texture *my_tex = DEMO->textureManager.texture[tex.ID];
			my_tex->shaderName = my_tex->type + std::to_string(i + 1);
			textures.push_back(tex);
		}
	}
	return textures;
}