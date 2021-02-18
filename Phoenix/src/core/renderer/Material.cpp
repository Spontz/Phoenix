// Material.cpp
// Spontz Demogroup

#include "main.h"
#include "core/renderer/Texture.h"
#include "core/renderer/Material.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

namespace Phoenix {


	Material::Material()
		:
		name(""),
		colDiffuse(glm::vec3(0)),
		colSpecular(glm::vec3(0)),
		colAmbient(glm::vec3(0)),
		strenghtSpecular(1),
		m_pMaterial(nullptr),
		m_pScene(nullptr),
		m_ModelDirectory(""),
		m_ModelFilename("")
	{
	}

	void Material::Load(const aiMaterial* pMaterial, const aiScene* pScene, std::string modelDirectory, std::string modelFilename)
	{
		m_pMaterial = pMaterial;
		m_pScene = pScene;
		m_ModelDirectory = modelDirectory;
		m_ModelFilename = modelFilename;

		// we assume a convention for sampler names in the shaders. Each diffuse texture should be named
		// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
		// Same applies to other texture as the following list summarizes:
		// diffuse: texture_diffuseN
		// specular: texture_specularN
		// normal: texture_normalN
		// Diffuse maps
		std::vector<textureStack> diffuseMaps = loadTextures(aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		Logger::info(LogLevel::low, "  The mesh has %d diffuseMaps", diffuseMaps.size());
		// Specular maps
		std::vector<textureStack> specularMaps = loadTextures(aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		Logger::info(LogLevel::low, "  The mesh has %d specularMaps", specularMaps.size());
		// Ambient maps
		std::vector<textureStack> ambientMaps = loadTextures(aiTextureType_AMBIENT, "texture_ambient");
		textures.insert(textures.end(), ambientMaps.begin(), ambientMaps.end());
		Logger::info(LogLevel::low, "  The mesh has %d ambientMaps", ambientMaps.size());
		// Height maps
		std::vector<textureStack> heightMaps = loadTextures(aiTextureType_HEIGHT, "texture_height");
		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
		Logger::info(LogLevel::low, "  The mesh has %d heightMaps", heightMaps.size());
		// Normal maps
		std::vector<textureStack> normalMaps = loadTextures(aiTextureType_NORMALS, "texture_normal");
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
		Logger::info(LogLevel::low, "  The mesh has %d normalMaps", normalMaps.size());
		// Emissive maps
		std::vector<textureStack> emissiveMaps = loadTextures(aiTextureType_EMISSIVE, "texture_emissive");
		textures.insert(textures.end(), emissiveMaps.begin(), emissiveMaps.end());
		Logger::info(LogLevel::low, "  The mesh has %d emissiveMaps", emissiveMaps.size());
		// Roughness maps
		std::vector<textureStack> roughnessMaps = loadTextures(aiTextureType_DIFFUSE_ROUGHNESS, "texture_roughness");
		textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());
		Logger::info(LogLevel::low, "  The mesh has %d roughnessMaps", roughnessMaps.size());
		// Shininess maps
		std::vector<textureStack> shininessMaps = loadTextures(aiTextureType_SHININESS, "texture_shininess");
		textures.insert(textures.end(), shininessMaps.begin(), shininessMaps.end());
		Logger::info(LogLevel::low, "  The mesh has %d shininessMaps", shininessMaps.size());
		// Ambient Occlusion maps
		std::vector<textureStack> ambientOclussionMaps = loadTextures(aiTextureType_AMBIENT_OCCLUSION, "texture_ambientoclussion");
		textures.insert(textures.end(), ambientOclussionMaps.begin(), ambientOclussionMaps.end());
		Logger::info(LogLevel::low, "  The mesh has %d ambientoclussionMaps", ambientOclussionMaps.size());
		// Metallness maps
		std::vector<textureStack> metalnessMaps = loadTextures(aiTextureType_METALNESS, "texture_metalness");
		textures.insert(textures.end(), metalnessMaps.begin(), metalnessMaps.end());
		Logger::info(LogLevel::low, "  The mesh has %d metalnessMaps", metalnessMaps.size());
		// Unknown
		std::vector<textureStack> unknownMaps = loadTextures(aiTextureType_UNKNOWN, "texture_unknown");
		textures.insert(textures.end(), unknownMaps.begin(), unknownMaps.end());
		Logger::info(LogLevel::low, "  The mesh has %d unknownMaps", unknownMaps.size());

		// None
		std::vector<textureStack> noneMaps = loadTextures(aiTextureType_NONE, "texture_none");
		textures.insert(textures.end(), noneMaps.begin(), noneMaps.end());
		Logger::info(LogLevel::low, "  The mesh has %d noneMaps", noneMaps.size());


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
	std::vector<textureStack> Material::loadTextures(aiTextureType type, std::string typeName)
	{
		std::vector<textureStack> textures;
		unsigned int textureCount = m_pMaterial->GetTextureCount(type);

		for (unsigned int i = 0; i < textureCount; i++)
		{
			aiString filepath;
			std::string fullpath;
			ai_real blendFactor;
			aiTextureOp operation;
			m_pMaterial->GetTexture(type, i, &filepath, NULL, NULL, &blendFactor, &operation);
			auto storageType = getTextureStorageType(type);

			// Load the texture depending on the storage type
			switch (storageType)
			{
			case TextureStorageType::Disk:
			{
				if (0 == strcmp(filepath.C_Str(), "$texture_dummy.bmp"))				// Prevent a bug in assimp: In some cases, the texture by default is named "$texture_dummy.bmp"
					filepath = m_ModelFilename.substr(0, m_ModelFilename.find_last_of('.')) + ".jpg";	// In that case, we change this to "<model_name.jpg>"
				fullpath = m_ModelDirectory + "/" + filepath.C_Str();
				textureStack tex;
				tex.tex = DEMO->m_textureManager.addTexture(fullpath.c_str(), false, typeName);
				tex.blendOperation = operation;
				tex.strength = blendFactor;
				if (tex.tex) {
					tex.shaderName = typeName + std::to_string(i + 1);
					textures.push_back(tex);
				}
				break;
			}
			case TextureStorageType::EmbeddedCompressed:
			case TextureStorageType::EmbeddedNonCompressed:
			{
				aiTexture const* pTexture = m_pScene->GetEmbeddedTexture(filepath.C_Str());
				textureStack tex;
				if (storageType == TextureStorageType::EmbeddedCompressed)
					tex.tex = DEMO->m_textureManager.addTextureFromMem(reinterpret_cast<unsigned char*>(pTexture->pcData), pTexture->mWidth, false, typeName);
				else
					tex.tex = DEMO->m_textureManager.addTextureFromMem(reinterpret_cast<unsigned char*>(pTexture->pcData), pTexture->mWidth * pTexture->mHeight, false, typeName);
				tex.blendOperation = operation;
				tex.strength = blendFactor;
				if (tex.tex) {
					tex.shaderName = typeName + std::to_string(i + 1);
					textures.push_back(tex);
				}
				break;
			}
			case TextureStorageType::IndexCompressed:
			case TextureStorageType::IndexNonCompressed:
			{
				std::string s_filepath = filepath.C_Str();
				s_filepath.erase(std::remove(s_filepath.begin(), s_filepath.end(), '*'), s_filepath.end()); // For some reason, some formats include the "*" character. I'm removing it

				int index = std::stoi(s_filepath.c_str());
				if (index < 0 || index >= (int)m_pScene->mNumTextures) {
					Logger::error("Error loading indexed texture, the specified texture number [%d] is not available", index);
				}
				else {
					aiTexture const* pTexture = m_pScene->mTextures[index];
					textureStack tex;
					if (storageType == TextureStorageType::IndexCompressed)
						tex.tex = DEMO->m_textureManager.addTextureFromMem(reinterpret_cast<unsigned char*>(pTexture->pcData), pTexture->mWidth, false, typeName);
					else
						tex.tex = DEMO->m_textureManager.addTextureFromMem(reinterpret_cast<unsigned char*>(pTexture->pcData), pTexture->mWidth * pTexture->mHeight, false, typeName);
					tex.blendOperation = operation;
					tex.strength = blendFactor;
					if (tex.tex) {
						tex.shaderName = typeName + std::to_string(i + 1);
						textures.push_back(tex);
					}
				}
				break;
			}

			default:
				Logger::info(LogLevel::low, "Texture format not supported... skip!");
			}

		}
		return textures;
	}

	TextureStorageType Material::getTextureStorageType(aiTextureType textureType)
	{
		if (!m_pMaterial->GetTextureCount(textureType))
			return TextureStorageType::None;

		aiString path;
		m_pMaterial->GetTexture(textureType, 0, &path);
		std::string texturePath = path.C_Str();

		if (texturePath[0] == '*')
		{
			if (m_pScene->mTextures[0]->mHeight == 0)
				return TextureStorageType::IndexCompressed;
			else
				return TextureStorageType::IndexNonCompressed;
		}
		else if (auto pTexture = m_pScene->GetEmbeddedTexture(texturePath.c_str()))
		{
			if (pTexture->mHeight == 0)
				return TextureStorageType::EmbeddedCompressed;
			else
				return TextureStorageType::EmbeddedNonCompressed;
		}
		else if (texturePath.find('.') != std::string::npos)
			return TextureStorageType::Disk;

		return TextureStorageType::None;
	}
}