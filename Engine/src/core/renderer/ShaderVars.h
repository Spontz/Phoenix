// ShaderVars.h
// Spontz Demogroup

#pragma once

#include "main.h"
#include "core/drivers/mathdriver.h"

#include <unordered_map>

namespace Phoenix {

	struct varBase {
		std::string					name;
		GLint						loc = -1; // TODO: We can delete this, it is cached in Shader class
		std::shared_ptr<MathDriver> eva;
	};

	template <typename value_type> struct varBasic final : public varBase {
		value_type value;
	};

	using varFloat = varBasic<float>;
	using varVec2 = varBasic<glm::vec2>;
	using varVec3 = varBasic<glm::vec3>;
	using varVec4 = varBasic<glm::vec4>;
	using varMat3 = varBasic<glm::mat3>;
	using varMat4 = varBasic<glm::mat4>;

	// Sampler2D (TEXTURE) evaluation
	struct varSampler2D final : public varBase {
		SP_Texture	texture;		// Texture
		int32_t		fboNum;			// Fbo number
		int32_t		fboAttachment;	// Fbo attachment
		int32_t		texUnitID;		// Texture unit ID
		bool		isFBO;			// is the Sampler2D an FBO?
	};

	// SamplerCube (CUBEMAP) evaluation
	struct varSamplerCube final : public varBase {
		SP_Cubemap	cubemap;		// Cubemap
		int32_t		cubemapUnitID;	// Texture unit ID
	};

	class ShaderVars final {
	private:
		enum class VarType {
			FLOAT,
			VEC2,
			VEC3,
			VEC4,
			MAT3,
			MAT4,
			SAMPLER2D,
			SAMPLERCUBE,
			UNKNOWN
		};

		mutable std::unordered_map<std::string_view, VarType> m_VarTypeMap = {
			{"float", VarType::FLOAT},
			{"vec2", VarType::VEC2},
			{"vec3", VarType::VEC3},
			{"vec4", VarType::VEC4},
			{"mat3", VarType::MAT3},
			{"mat4", VarType::MAT4},
			{"sampler2D", VarType::SAMPLER2D},
			{"samplerCube", VarType::SAMPLERCUBE}
		};

	public:
		std::vector<std::shared_ptr<varFloat>>			vfloat;
		std::vector<std::shared_ptr<varVec2>>			vec2;
		std::vector<std::shared_ptr<varVec3>>			vec3;
		std::vector<std::shared_ptr<varVec4>>			vec4;
		std::vector<std::shared_ptr<varMat3>>			mat3;
		std::vector<std::shared_ptr<varMat4>>			mat4;
		std::vector<std::shared_ptr<varSampler2D>>		sampler2D;
		std::vector<std::shared_ptr<varSamplerCube>>	samplerCube;

		// constructor generates the shader on the fly
		ShaderVars(Section* pSection, SP_Shader const& spShader);
		bool ReadString(std::string_view stringVar); // Reads a string that contains vars and add it
		void setValues();

	private:
		SP_Shader my_shader;	// Shader where the vars are poiting
		Section* my_section;	// Section where the vars are needed (required for formula variables, like time, etc.)

		size_t splitString(const std::string& txt, std::vector<std::string>& strs, char ch);
		bool loadTextureProperty(Texture::Properties& texProperty, const std::string& property);
		bool loadFboProperty(int32_t& colorAttachments, const std::string& property);
		VarType getVarType(std::string_view varTypeStr) const;
		
	};
}