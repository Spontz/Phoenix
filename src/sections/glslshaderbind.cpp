#include "main.h"

// ******************************************************************
#define MAX_SHADER_VARS 128			// Max shader variables that can be stored in one shader
#define MAXSIZE_VAR_NAME 128			// Max size of a variable name
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
	int			texture;	// Engine internal texture ID
	int			texGLid;	// Texture ID (for binding it)
} varSampler2D;				// Structure for a evaluation Sampler2D (TEXTURE)

typedef struct {
	int				shader;

	varFloat		vfloat[MAX_SHADER_VARS];
	int				vfloat_num;
	varVec2			vec2[MAX_SHADER_VARS];
	int				vec2_num;
	varVec3			vec3[MAX_SHADER_VARS];
	int				vec3_num;
	varVec4			vec4[MAX_SHADER_VARS];
	int				vec4_num;
	varSampler2D	sampler2D[MAX_SHADER_VARS];
	int				sampler2D_num;
} glslshaderbind_section;

static glslshaderbind_section *local;

// ******************************************************************

static int max_shader_reached(int val) {
	if (val >= (MAX_SHADER_VARS - 1)) {
		LOG->Error("GLSLShaderBind: Too many variables (%d)! MAX_SHADER_VARS (%d) reached!! you need to recompile the engine or use less variables!", val, MAX_SHADER_VARS);
		val = (MAX_SHADER_VARS - 1);
	}
	return val;
}

// ******************************************************************

sGLSLShaderBind::sGLSLShaderBind()
{
	type = SectionType::GLSLShaderBind;
}

void sGLSLShaderBind::load() {
	int		i;
	int		num;
	char	var_name[MAXSIZE_VAR_NAME];
	char	var_type[MAXSIZE_VAR_TYPE];
	char	var_value[MAXSIZE_VAR_EVAL];

	string s_demo = DEMO->demoDir;

	Shader*			my_shader;
	varFloat*		vfloat;
	varVec2*		vec2;
	varVec3*		vec3;
	varVec4*		vec4;
	varSampler2D*	sampler2D;
	

	// script validation
	// 2 strings needed: Vertex and fragment shader path
	if (this->stringNum < 2) {
		LOG->Error("GLSLShaderBind [%s]: 2 strings are needed: vertex and fragment shader files", this->identifier.c_str());
		return;
	}

	local = (glslshaderbind_section*) malloc(sizeof(glslshaderbind_section));
	this->vars = (void *)local;

	// load Shader
	local->shader = DEMO->shaderManager.addShader(s_demo + this->strings[0], s_demo + this->strings[1]);

	if (local->shader == -1)
		return;
	
	my_shader = DEMO->shaderManager.shader[local->shader];
	my_shader->use();

	// Reset variables
	local->vfloat_num = 0;
	local->vec2_num = 0;
	local->vec3_num = 0;
	local->vec4_num = 0;
	local->sampler2D_num = 0;
	num = 0;

	// Read the variables
	for (i = 2; i < this->stringNum; i++) {
		sscanf(this->strings[i].c_str(), "%s %s %s", var_type, var_name, var_value);	// Read one string and store values on temporary strings for further evaluation
		LOG->Info(LOG_MED,"GLSLShaderBind [%s]: string_type [%s], string_name [%s], string_value [%s]", this->identifier.c_str(), var_type, var_name, var_value);

		if (strcmp(var_type, "float") == 0)	// FLOAT detected
		{
			num = max_shader_reached(local->vfloat_num++);
			vfloat = &(local->vfloat[num]);
			strcpy(vfloat->name, var_name);
			vfloat->loc = my_shader->getUniformLocation(vfloat->name);
			vfloat->eva = new mathDriver(this);
			vfloat->eva->expression = var_value;
			vfloat->eva->SymbolTable.add_variable("v1", vfloat->value);
			vfloat->eva->compileFormula();
			
		}
		else if (strcmp(var_type, "vec2") == 0)	// VEC2 detected
		{
			num = max_shader_reached(local->vec2_num++);
			vec2 = &(local->vec2[num]);
			strcpy(vec2->name, var_name);
			vec2->loc = my_shader->getUniformLocation(vec2->name);
			vec2->eva = new mathDriver(this);
			vec2->eva->expression = var_value;
			vec2->eva->SymbolTable.add_variable("v1", vec2->value[0]);
			vec2->eva->SymbolTable.add_variable("v2", vec2->value[1]);
			vec2->eva->compileFormula();
		}
		else if (strcmp(var_type, "vec3") == 0)	// VEC3 detected
		{
			num = max_shader_reached(local->vec3_num++);
			vec3 = &(local->vec3[num]);
			strcpy(vec3->name, var_name);
			vec3->loc = my_shader->getUniformLocation(vec3->name);
			vec3->eva = new mathDriver(this);
			vec3->eva->expression = var_value;
			vec3->eva->SymbolTable.add_variable("v1", vec3->value[0]);
			vec3->eva->SymbolTable.add_variable("v2", vec3->value[1]);
			vec3->eva->SymbolTable.add_variable("v3", vec3->value[2]);
			vec3->eva->compileFormula();
		}
		else if (strcmp(var_type, "vec4") == 0)	// VEC4 detected
		{
			num = max_shader_reached(local->vec4_num++);
			vec4 = &(local->vec4[num]);
			strcpy(vec4->name, var_name);
			vec4->loc = my_shader->getUniformLocation(vec4->name);
			vec4->eva = new mathDriver(this);
			vec4->eva->expression = var_value;
			vec4->eva->SymbolTable.add_variable("v1", vec4->value[0]);
			vec4->eva->SymbolTable.add_variable("v2", vec4->value[1]);
			vec4->eva->SymbolTable.add_variable("v3", vec4->value[2]);
			vec4->eva->SymbolTable.add_variable("v4", vec4->value[3]);
			vec4->eva->compileFormula();
		}
		else if (strcmp(var_type, "sampler2D") == 0)	// Texture (sampler2D) detected
		{
			num = max_shader_reached(local->sampler2D_num++);
			sampler2D = &(local->sampler2D[num]);
			strcpy(sampler2D->name, var_name);
			sampler2D->loc = my_shader->getUniformLocation(sampler2D->name);
			// If sampler2D is a fbo...
			if (0 == strncmp("fbo", var_value, 3)) {
				int fbonum;
				sscanf(var_value, "fbo%d", &fbonum);
				if (fbonum<0 || fbonum>(FBO_BUFFERS - 1)) {
					LOG->Error("GLSLShaderBind [%s]: sampler2D fbo not correct, it should be 'fboX', where X=>0 and X<=%d, you choose: %s", this->identifier.c_str(), (FBO_BUFFERS - 1), var_value);
					return;
				}
				else {
					sampler2D->texture = fbonum;
					sampler2D->texGLid = DEMO->fboManager.getOpenGLTextureID(fbonum);
				}
			}
			// Is it s a normal texture...
			else {
				sampler2D->texture = DEMO->textureManager.addTexture(s_demo + var_value);
				sampler2D->texGLid = DEMO->textureManager.getOpenGLTextureID(sampler2D->texture);
			}
		}
	}

	// Unbind any shader used
	DEMO->shaderManager.unbindShaders();
}

void sGLSLShaderBind::init() {
	
}

void sGLSLShaderBind::exec() {
	varFloat*		vfloat;
	varVec2*		vec2;
	varVec3*		vec3;
	varVec4*		vec4;
	varSampler2D*	sampler2D;
	int				i;

	local = (glslshaderbind_section*)this->vars;

	if (local->shader == -1)
		return;

	// Choose the proper program shader
	Shader *my_shader = DEMO->shaderManager.shader[local->shader];
	my_shader->use();
	
	for (i = 0; i < local->vfloat_num; i++) {
		vfloat = &(local->vfloat[i]);
		vfloat->eva->Expression.value();
		my_shader->setValue(vfloat->name, vfloat->value);
	}

	for (i = 0; i < local->vec2_num; i++) {
		vec2 = &(local->vec2[i]);
		vec2->eva->Expression.value();
		my_shader->setValue(vec2->name, glm::vec2(vec2->value[0], vec2->value[1]));
	}

	for (i = 0; i < local->vec3_num; i++) {
		vec3 = &(local->vec3[i]);
		vec3->eva->Expression.value();
		my_shader->setValue(vec3->name, glm::vec3(vec3->value[0], vec3->value[1], vec3->value[2]));
	}

	for (i = 0; i < local->vec4_num; i++) {
		vec4 = &(local->vec4[i]);
		vec4->eva->Expression.value();
		my_shader->setValue(vec4->name, glm::vec4(vec4->value[0], vec4->value[1], vec4->value[2], vec4->value[3]));
	}

	for (i = local->sampler2D_num - 1; i >= 0; i--) {
		sampler2D = &(local->sampler2D[i]);
		my_shader->setValue(sampler2D->name, sampler2D->texGLid);
	}
}

void sGLSLShaderBind::end() {
	
}
