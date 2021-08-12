// Material.h
// Spontz Demogroup

#pragma once

#include "main.h"
#include "core/Demokernel.h"
#include "core/renderer/Texture.h"

namespace Phoenix {

	struct textureStack final
	{
		SP_Texture tex;			// Texture
		std::string	shaderName;		// Name that receives the texture in the shader
		uint32_t	blendOperation=0;	// Blend operation with the base color
		float		strength=0.;		// Strength factor
	};

	enum class TextureStorageType
	{
		None = 0, Disk, IndexCompressed, IndexNonCompressed, EmbeddedCompressed, EmbeddedNonCompressed
	};

	// Specs: http://assimp.sourceforge.net/lib_html/materials.html
	class Material final {
	public:
		std::string					name;
		std::vector<textureStack>	textures;
		glm::vec3					colDiffuse;
		glm::vec3					colSpecular;
		glm::vec3					colAmbient;
		float						strenghtSpecular;
	private:
		const aiMaterial* m_pMaterial;
		const aiScene* m_pScene;
		std::string					m_ModelDirectory;	// Path of the model file
		std::string					m_ModelFilename;	// Name of the model file

	public:
		Material();
		void Load(const aiMaterial* pMaterial, const aiScene* pScene, std::string modelDirectory, std::string modelFilename);

	private:
		std::vector<textureStack> loadTextures(aiTextureType type, std::string typeName);
		TextureStorageType getTextureStorageType(aiTextureType textureType);

	};
}