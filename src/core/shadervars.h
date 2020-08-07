// shadervars.h
// Spontz Demogroup

#ifndef SHADERVARS_H
#define SHADERVARS_H

#include "main.h"

#define MAXSIZE_VAR_NAME 128		// Max size of a variable name
#define MAXSIZE_VAR_TYPE 64			// Max size of a variable type
#define MAXSIZE_VAR_EVAL 512		// Max size of a variable equation

typedef struct {
	char		name[MAXSIZE_VAR_NAME];
	GLint		loc;
	float		value;
	mathDriver	*eva;
} varFloat;				// Structure for a evaluation FLOAT

typedef struct {
	char		name[MAXSIZE_VAR_NAME];
	GLint		loc;
	glm::vec2	value;
	mathDriver	*eva;
} varVec2;				// Structure for a evaluation VEC2

typedef struct {
	char		name[MAXSIZE_VAR_NAME];
	GLint		loc;
	glm::vec3	value;
	mathDriver	*eva;
} varVec3;				// Structure for a evaluation VEC3

typedef struct {
	char		name[MAXSIZE_VAR_NAME];
	GLint		loc;
	glm::vec4	value;
	mathDriver	*eva;
} varVec4;				// Structure for a evaluation VEC4

typedef struct {
	char		name[MAXSIZE_VAR_NAME];
	GLint		loc;
	glm::mat3	value;
	mathDriver	*eva;
} varMat3;				// Structure for a evaluation MAT3

typedef struct {
	char		name[MAXSIZE_VAR_NAME];
	GLint		loc;
	glm::mat4	value;
	mathDriver	*eva;
} varMat4;				// Structure for a evaluation MAT4

typedef struct {
	char			name[MAXSIZE_VAR_NAME];
	GLint			loc;
	int				texture;	// Engine internal texture ID
	int				texGLid;	// Texture ID
	int				texUnitID;	// Texture unit ID
	bool			isFBO;		// is the Sampler2D an FBO?
} varSampler2D;				// Structure for a evaluation Sampler2D (TEXTURE)


class ShaderVars
{
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
	bool ReadString(const char *string_var); // Reads a string that contains vars and add it
	void setValues();

private:
	Shader* my_shader;		// Shader where the vars are poiting
	Section* my_section;	// Section where the vars are needed (required for formula variables, like time, etc.)

	size_t splitString(const std::string &txt, std::vector<std::string> &strs, char ch);


};
#endif