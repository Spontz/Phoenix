// material.cpp
// Spontz Demogroup

#include "main.h"
#include "core/texture.h"
#include "core/material.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

Material::Material(aiMaterial *pMaterial, string modelDirectory, string modelFilename)
{
	this->m_pMaterial = pMaterial;
	this->m_ModelDirectory = modelDirectory;
	this->m_ModelFilename = modelFilename;


	// 1. diffuse maps
	vector<int> diffuseMaps = loadTextures(pMaterial, aiTextureType_DIFFUSE, "texture_diffuse");
	textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
	LOG->Info(LOG_LOW, "  The mesh has %d diffuseMaps", diffuseMaps.size());
	// 2. specular maps
	vector<int> specularMaps = loadTextures(pMaterial, aiTextureType_SPECULAR, "texture_specular");
	textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	LOG->Info(LOG_LOW, "  The mesh has %d specularMaps", specularMaps.size());
	// 3. normal maps
	std::vector<int> normalMaps = loadTextures(pMaterial, aiTextureType_HEIGHT, "texture_normal");
	textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
	LOG->Info(LOG_LOW, "  The mesh has %d normalMaps", normalMaps.size());
	// 4. height maps
	std::vector<int> heightMaps = loadTextures(pMaterial, aiTextureType_AMBIENT, "texture_height");
	textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
	LOG->Info(LOG_LOW, "  The mesh has %d heightMaps", heightMaps.size());
	// Unknown
	std::vector<int> unknownMaps = loadTextures(pMaterial, aiTextureType_NONE, "texture_unknown");
	textures.insert(textures.end(), unknownMaps.begin(), unknownMaps.end());
	LOG->Info(LOG_LOW, "  The mesh has %d unknownMaps", unknownMaps.size());

	aiColor3D color(0.f, 0.f, 0.f);
	pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);
}


// checks all material textures of a given type and loads the textures if they're not loaded yet.
// the required info is returned as a Texture struct.
vector<int> Material::loadTextures(aiMaterial *mat, aiTextureType type, string typeName)
{
	vector<int> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString filepath;
		string fullpath;
		mat->GetTexture(type, i, &filepath);
		if (0 == strcmp(filepath.C_Str(), "$texture_dummy.bmp"))				// Prevent a bug in assimp: In some cases, the texture by default is named "$texture_dummy.bmp"
			filepath = m_ModelFilename.substr(0, m_ModelFilename.find_last_of('.')) + ".jpg";	// In that case, we change this to "<model_name.jpg>"
		fullpath = m_ModelDirectory + "/" + filepath.C_Str();
		int tex = DEMO->textureManager.addTexture(fullpath.c_str(), false, typeName);
		textures.push_back(tex);
	}
	return textures;
}