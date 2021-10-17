// Material.cpp
// Spontz Demogroup

#include "main.h"
#include "core/renderer/Texture.h"
#include "core/renderer/Material.h"

namespace Phoenix {

	Material::Material()
		:
		colDiffuse(glm::vec3(0)),
		colSpecular(glm::vec3(0)),
		colAmbient(glm::vec3(0)),
		strenghtSpecular(1),
		m_pMaterial(nullptr),
		m_pScene(nullptr)
	{
	}

	void Material::Load(
		const aiMaterial* pMaterial,
		const aiScene* pScene,
		std::string_view modelDirectory,
		std::string_view modelFilename
	)
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

		Logger::ScopedIndent _;

		// Diffuse maps
		const auto diffuseMaps = loadTextures(aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		Logger::info(LogLevel::low, "The mesh has {} diffuseMaps", diffuseMaps.size());
		// Specular maps
		const auto specularMaps = loadTextures(aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		Logger::info(LogLevel::low, "The mesh has {} specularMaps", specularMaps.size());
		// Ambient maps
		const auto ambientMaps = loadTextures(aiTextureType_AMBIENT, "texture_ambient");
		textures.insert(textures.end(), ambientMaps.begin(), ambientMaps.end());
		Logger::info(LogLevel::low, "The mesh has {} ambientMaps", ambientMaps.size());
		// Height maps
		const auto heightMaps = loadTextures(aiTextureType_HEIGHT, "texture_height");
		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
		Logger::info(LogLevel::low, "The mesh has {} heightMaps", heightMaps.size());
		// Normal maps
		const auto normalMaps = loadTextures(aiTextureType_NORMALS, "texture_normal");
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
		Logger::info(LogLevel::low, "The mesh has {} normalMaps", normalMaps.size());
		// Emissive maps
		const auto emissiveMaps = loadTextures(aiTextureType_EMISSIVE, "texture_emissive");
		textures.insert(textures.end(), emissiveMaps.begin(), emissiveMaps.end());
		Logger::info(LogLevel::low, "The mesh has {} emissiveMaps", emissiveMaps.size());
		// Roughness maps
		const auto roughnessMaps = loadTextures(aiTextureType_DIFFUSE_ROUGHNESS, "texture_roughness");
		textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());
		Logger::info(LogLevel::low, "The mesh has {} roughnessMaps", roughnessMaps.size());
		// Shininess maps
		const auto shininessMaps = loadTextures(aiTextureType_SHININESS, "texture_shininess");
		textures.insert(textures.end(), shininessMaps.begin(), shininessMaps.end());
		Logger::info(LogLevel::low, "The mesh has {} shininessMaps", shininessMaps.size());
		// Ambient Occlusion maps
		const auto ambientOclussionMaps = loadTextures(aiTextureType_AMBIENT_OCCLUSION, "texture_ambientoclussion");
		textures.insert(textures.end(), ambientOclussionMaps.begin(), ambientOclussionMaps.end());
		Logger::info(LogLevel::low, "The mesh has {} ambientoclussionMaps", ambientOclussionMaps.size());
		// Metallness maps
		const auto metalnessMaps = loadTextures(aiTextureType_METALNESS, "texture_metalness");
		textures.insert(textures.end(), metalnessMaps.begin(), metalnessMaps.end());
		Logger::info(LogLevel::low, "The mesh has {} metalnessMaps", metalnessMaps.size());
		// Unknown
		const auto unknownMaps = loadTextures(aiTextureType_UNKNOWN, "texture_unknown");
		textures.insert(textures.end(), unknownMaps.begin(), unknownMaps.end());
		Logger::info(LogLevel::low, "The mesh has {} unknownMaps", unknownMaps.size());
		// None
		const auto noneMaps = loadTextures(aiTextureType_NONE, "texture_none");
		textures.insert(textures.end(), noneMaps.begin(), noneMaps.end());
		Logger::info(LogLevel::low, "The mesh has {} noneMaps", noneMaps.size());

		aiColor3D color;

		// Diffuse color
		pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		colDiffuse = glm::vec3(color.r, color.g, color.b);

		// Ambient color
		pMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color);
		colAmbient = glm::vec3(color.r, color.g, color.b);

		// Specular: Specular color = COLOR_SPECULAR * SHININESS_STRENGTH
		pMaterial->Get(AI_MATKEY_SHININESS_STRENGTH, strenghtSpecular);
		pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color);
		colSpecular = glm::vec3(color.r, color.g, color.b);
	}

	// checks all material textures of a given type and loads the textures if they're not loaded yet.
	// the required info is returned as a Texture struct.
	std::vector<textureStack> Material::loadTextures(aiTextureType type, std::string_view typeName)
	{
		std::vector<textureStack> textures;
		const auto textureCount = m_pMaterial->GetTextureCount(type);

		for (unsigned int i = 0; i < textureCount; ++i) {
			aiString filepath;
			ai_real blendFactor;
			aiTextureOp operation;
			m_pMaterial->GetTexture(type, i, &filepath, NULL, NULL, &blendFactor, &operation);
			const auto storageType = getTextureStorageType(type);

			// Load the texture depending on the storage type
			switch (storageType)
			{
			case TextureStorageType::Disk:
			{
				// Prevent a bug in assimp: In some cases, the texture by default is named "$texture_dummy.bmp"
				if (0 == strcmp(filepath.C_Str(), "$texture_dummy.bmp")) {
					// In that case, we change this to "<model_name.jpg>"
					filepath = m_ModelFilename.substr(0, m_ModelFilename.find_last_of('.')) + ".jpg";
				}
				const auto fullpath = m_ModelDirectory + "/" + filepath.C_Str();
				Texture::Properties texProps;
				texProps.m_flip = false;
				texProps.m_type = typeName;
				textureStack tex;
				tex.tex = DEMO->m_textureManager.addTexture(fullpath, texProps);
				tex.blendOperation = operation;
				tex.strength = blendFactor;
				if (tex.tex) {
					tex.shaderName = std::string(typeName) + std::to_string(i + 1);
					textures.push_back(tex);
				}
				break;
			}
			case TextureStorageType::EmbeddedCompressed:
			case TextureStorageType::EmbeddedNonCompressed:
			{
				aiTexture const* pTexture = m_pScene->GetEmbeddedTexture(filepath.C_Str());
				Texture::Properties texProps;
				texProps.m_flip = false;
				texProps.m_type = typeName;

				textureStack tex;
				if (storageType == TextureStorageType::EmbeddedCompressed)
					tex.tex = DEMO->m_textureManager.addTextureFromMem(reinterpret_cast<unsigned char*>(pTexture->pcData), pTexture->mWidth, texProps);
				else
					tex.tex = DEMO->m_textureManager.addTextureFromMem(reinterpret_cast<unsigned char*>(pTexture->pcData), pTexture->mWidth * pTexture->mHeight, texProps);
				tex.blendOperation = operation;
				tex.strength = blendFactor;
				if (tex.tex) {
					tex.shaderName = std::string(typeName) + std::to_string(i + 1);
					textures.push_back(tex);
				}
				break;
			}
			case TextureStorageType::IndexCompressed:
			case TextureStorageType::IndexNonCompressed:
			{
				std::string s_filepath = filepath.C_Str();
				s_filepath.erase(std::remove(s_filepath.begin(), s_filepath.end(), '*'), s_filepath.end()); // We need to remove the '*' character in order to get the index number

				int index = std::stoi(s_filepath.c_str());
				if (index < 0 || index >= (int)m_pScene->mNumTextures) {
					Logger::error("Error loading indexed texture, the specified texture number [{}] is not available", index);
				}
				else {
					aiTexture const* pTexture = m_pScene->mTextures[index];
					Texture::Properties texProps;
					texProps.m_flip = false;
					texProps.m_type = typeName;

					textureStack tex;
					if (storageType == TextureStorageType::IndexCompressed)
						tex.tex = DEMO->m_textureManager.addTextureFromMem(reinterpret_cast<unsigned char*>(pTexture->pcData), pTexture->mWidth, texProps);
					else
						tex.tex = DEMO->m_textureManager.addTextureFromMem(reinterpret_cast<unsigned char*>(pTexture->pcData), pTexture->mWidth * pTexture->mHeight, texProps);
					tex.blendOperation = operation;
					tex.strength = blendFactor;
					if (tex.tex) {
						tex.shaderName = std::string(typeName) + std::to_string(i + 1);
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

	TextureStorageType Material::getTextureStorageType(aiTextureType textureType) const
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
