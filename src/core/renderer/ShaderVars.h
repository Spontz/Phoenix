// ShaderVars.h
// Spontz Demogroup

#pragma once

#include "main.h"
#include "core/drivers/mathdriver.h"

static constexpr size_t MAXSIZE_VAR_NAME = 128; // Max size of a variable name
static constexpr size_t MAXSIZE_VAR_TYPE = 64;  // Max size of a variable type
static constexpr size_t MAXSIZE_VAR_EVAL = 512; // Max size of a variable equation

struct varBase {
    char		name[MAXSIZE_VAR_NAME] = "\0";
    GLint		loc = -1;
    MathDriver* eva = nullptr;
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
    Texture* texture;   // Texture
    int      fboNum;    // Fbo number
    int      texUnitID; // Texture unit ID
    bool     isFBO;     // is the Sampler2D an FBO?
};

class ShaderVars final {
public:
    std::vector<varFloat*>		vfloat;
    std::vector<varVec2*>		vec2;
    std::vector<varVec3*>		vec3;
    std::vector<varVec4*>		vec4;
    std::vector<varMat3*>		mat3;
    std::vector<varMat4*>		mat4;
    std::vector<varSampler2D*>	sampler2D;

    // constructor generates the shader on the fly
    ShaderVars(Section* sec, Shader* shad);
    bool ReadString(const char* string_var); // Reads a string that contains vars and add it
    void setValues();

private:
    Shader* my_shader;		// Shader where the vars are poiting
    Section* my_section;	// Section where the vars are needed (required for formula variables, like time, etc.)

    size_t splitString(const std::string& txt, std::vector<std::string>& strs, char ch);
};
