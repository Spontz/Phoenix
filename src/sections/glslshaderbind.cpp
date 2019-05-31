#include "main.h"
#include "core/shadervars.h"

typedef struct {
	int				shader;
	ShaderVars		*vars;
} glslshaderbind_section;

static glslshaderbind_section *local;

// ******************************************************************

sGLSLShaderBind::sGLSLShaderBind() {
	type = SectionType::GLSLShaderBind;
}

bool sGLSLShaderBind::load() {
	
	string s_demo = DEMO->demoDir;
	Shader*			my_shader;

	// script validation
	// 2 strings needed: Vertex and fragment shader path
	if (this->stringNum < 2) {
		LOG->Error("GLSLShaderBind [%s]: 2 strings are needed: vertex and fragment shader files", this->identifier.c_str());
		return false;
	}

	local = (glslshaderbind_section*) malloc(sizeof(glslshaderbind_section));
	this->vars = (void *)local;

	// load Shader
	local->shader = DEMO->shaderManager.addShader(s_demo + this->strings[0], s_demo + this->strings[1]);
	if (local->shader == -1)
		return false;
	
	my_shader = DEMO->shaderManager.shader[local->shader];
	my_shader->use();

	// Create variables
	local->vars = new ShaderVars(this, my_shader);
	
	// Read the variables
	for (int i = 2; i < this->stringNum; i++) {
		local->vars->ReadString(this->strings[i].c_str());
	}

	// Unbind any shader used
	DEMO->shaderManager.unbindShaders();
	return true;
}

void sGLSLShaderBind::init() {
	
}

void sGLSLShaderBind::exec() {
	int i;
	varFloat*		vfloat;
	varVec2*		vec2;
	varVec3*		vec3;
	varVec4*		vec4;
	varMat3*		mat3;
	varMat4*		mat4;
	varSampler2D*	sampler2D;

	local = (glslshaderbind_section*)this->vars;
	if (local->shader == -1)
		return;

	// Choose the proper program shader
	Shader *my_shader = DEMO->shaderManager.shader[local->shader];
	my_shader->use();
	
	for (i = 0; i < local->vars->vfloat.size(); i++) {
		vfloat = local->vars->vfloat[i];
		vfloat->eva->Expression.value();
		my_shader->setValue(vfloat->name, vfloat->value);
	}

	for (i = 0; i < local->vars->vec2.size(); i++) {
		vec2 = local->vars->vec2[i];
		vec2->eva->Expression.value();
		my_shader->setValue(vec2->name, glm::vec2(vec2->value[0], vec2->value[1]));
	}

	for (i = 0; i < local->vars->vec3.size(); i++) {
		vec3 = local->vars->vec3[i];
		vec3->eva->Expression.value();
		my_shader->setValue(vec3->name, glm::vec3(vec3->value[0], vec3->value[1], vec3->value[2]));
	}

	for (i = 0; i < local->vars->vec4.size(); i++) {
		vec4 = local->vars->vec4[i];
		vec4->eva->Expression.value();
		my_shader->setValue(vec4->name, glm::vec4(vec4->value[0], vec4->value[1], vec4->value[2], vec4->value[3]));
	}

	for (i = 0; i < local->vars->mat3.size(); i++) {
		mat3 = local->vars->mat3[i];
		mat3->eva->Expression.value();
		// TODO: Probar si esto funciona asi:
		//my_shader->setValue(mat3->name, glm::mat3(mat3->value[0]));
		my_shader->setValue(mat3->name, glm::mat3(	mat3->value[0], mat3->value[1], mat3->value[2],
													mat3->value[3], mat3->value[4], mat3->value[5],
													mat3->value[6], mat3->value[7], mat3->value[8]));
	}

	for (i = 0; i < local->vars->mat4.size(); i++) {
		mat4 = local->vars->mat4[i];
		mat4->eva->Expression.value();
		my_shader->setValue(mat4->name, glm::mat4(	mat4->value[0],  mat4->value[1],  mat4->value[2],  mat4->value[3],
													mat4->value[4],  mat4->value[5],  mat4->value[6],  mat4->value[7], 
													mat4->value[8],  mat4->value[9],  mat4->value[10], mat4->value[11], 
													mat4->value[12], mat4->value[13], mat4->value[14], mat4->value[15]));
	}

	for (i = (int)local->vars->sampler2D.size() - 1; i >= 0; i--) {
		sampler2D = local->vars->sampler2D[i];
		my_shader->setValue(sampler2D->name, sampler2D->texGLid);
	}
}

void sGLSLShaderBind::end() {
	
}
