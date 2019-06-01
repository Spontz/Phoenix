// shadervars.h
// Spontz Demogroup

#ifndef SHADERVARS_H
#define SHADERVARS_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "main.h"
#include "core/shader.h"


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
	float		value[2];
	mathDriver	*eva;
} varVec2;				// Structure for a evaluation VEC2

typedef struct {
	char		name[MAXSIZE_VAR_NAME];
	GLint		loc;
	float		value[3];
	mathDriver	*eva;
} varVec3;				// Structure for a evaluation VEC3

typedef struct {
	char		name[MAXSIZE_VAR_NAME];
	GLint		loc;
	float		value[4];
	mathDriver	*eva;
} varVec4;				// Structure for a evaluation VEC4

typedef struct {
	char		name[MAXSIZE_VAR_NAME];
	GLint		loc;
	float		value[9];
	mathDriver	*eva;
} varMat3;				// Structure for a evaluation MAT3

typedef struct {
	char		name[MAXSIZE_VAR_NAME];
	GLint		loc;
	float		value[16];
	mathDriver	*eva;
} varMat4;				// Structure for a evaluation MAT4

typedef struct {
	char			name[MAXSIZE_VAR_NAME];
	GLint			loc;
	int				texture;	// Engine internal texture ID
	int				texGLid;	// Texture ID (for binding it)
	int				texUnitID;	// Texture unit ID (for glActiveTexture)
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
	

private:
	Shader* my_shader;		// Shader where the vars are poiting
	Section* my_section;	// Section where the vars are needed (required for formula variables, like time, etc.)

};
#endif