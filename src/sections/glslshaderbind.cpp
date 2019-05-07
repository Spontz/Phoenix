#include "main.h"

// ******************************************************************
#define MAX_SHADER_VARS 128	// Max shader variables that can be stored in one shader

typedef struct {
	string		name;
	float		value;
	mathDriver	*eva;
} varFloat;				// Structure for a evaluation FLOAT

typedef struct {
	string		name;
	float		value[2];
	mathDriver	*eva;
} varVec2;				// Structure for a evaluation VEC2

typedef struct {
	string		name;
	float		value[3];
	mathDriver	*eva;
} varVec3;				// Structure for a evaluation VEC3

typedef struct {
	string		name;
	float		value[4];
	mathDriver	*eva;
} varVec4;				// Structure for a evaluation VEC4

typedef struct {
	string		name;
	int			texture;	// Engine internal texture ID
	int			texGLid;	// Texture ID (for binding it)
} varSampler2D;				// Structure for a evaluation Sampler2D (TEXTURE)

typedef struct {
	int							shader;

	varFloat					vfloat[MAX_SHADER_VARS];
	int							vfloat_num;

	varVec2						vec2[MAX_SHADER_VARS];
	int							vec2_num;

	varVec3						vec3[MAX_SHADER_VARS];
	int							vec3_num;

	varVec4						vec4[MAX_SHADER_VARS];
	int							vec4_num;

	varSampler2D				sampler2D[MAX_SHADER_VARS];
	int							sampler2D_num;
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
	char	var_name[128];
	char	var_type[128];
	char	var_value[512];

	string	string_name;
	string	string_type;
	string	string_value;

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
		string_type = var_type;
		string_name = var_name;
		string_value = var_value;
		LOG->Info(LOG_MED,"GLSLShaderBind [%s]: string_type [%s], string_name [%s], string_value [%s]", this->identifier.c_str(), string_type.c_str(), string_name.c_str(), string_value.c_str());

		if (string_type == "float")	// FLOAT detected
		{
			num = max_shader_reached(local->vfloat_num++);
			vfloat = &(local->vfloat[num]);
			vfloat->name = string_name;
			vfloat->eva = new mathDriver(this);
			vfloat->eva->expression = string_value;
			vfloat->eva->SymbolTable.add_variable("v1", vfloat->value);
		}
		else if (string_type == "vec2")	// VEC2 detected
		{
			num = max_shader_reached(local->vec2_num++);
			vec2 = &(local->vec2[num]);
			vec2->name = string_name;
			vec2->eva = new mathDriver(this);
			vec2->eva->expression = string_value;
			vfloat->eva->SymbolTable.add_variable("v1", vec2->value[0]);
			vfloat->eva->SymbolTable.add_variable("v2", vec2->value[1]);
		}
		else if (string_type == "vec3")	// VEC3 detected
		{
			num = max_shader_reached(local->vec3_num++);
			vec3 = &(local->vec3[num]);
			vec3->name = string_name;
			vec3->eva = new mathDriver(this);
			vec3->eva->expression = string_value;
			vfloat->eva->SymbolTable.add_variable("v1", vec3->value[0]);
			vfloat->eva->SymbolTable.add_variable("v2", vec3->value[1]);
			vfloat->eva->SymbolTable.add_variable("v3", vec3->value[2]);
		}
		else if (string_type == "vec4")	// VEC4 detected
		{
			num = max_shader_reached(local->vec4_num++);
			vec4 = &(local->vec4[num]);
			vec4->name = string_name;
			vec4->eva = new mathDriver(this);
			vec4->eva->expression = string_value;
			vfloat->eva->SymbolTable.add_variable("v1", vec4->value[0]);
			vfloat->eva->SymbolTable.add_variable("v2", vec4->value[1]);
			vfloat->eva->SymbolTable.add_variable("v3", vec4->value[2]);
			vfloat->eva->SymbolTable.add_variable("v4", vec4->value[3]);
		}
		else if (string_type == "sampler2D")	// Texture (sampler2D) detected
		{
			num = max_shader_reached(local->sampler2D_num++);
			sampler2D = &(local->sampler2D[num]);
			sampler2D->name = string_name;
			// If sampler2D is a fbo...
			if (string_value.find("fbo")) {
				int fbonum;
				sscanf(string_value.c_str(), "fbo%d", &fbonum);
				if (fbonum<0 || fbonum>(FBO_BUFFERS - 1)) {
					LOG->Error("GLSLShaderBind [%s]: sampler2D fbo not correct, it should be 'fboX', where X=>0 and X<=%d, you choose: %s", this->identifier.c_str(), (FBO_BUFFERS - 1), string_value.c_str());
					return;
				}
				else {
					sampler2D->texture = fbonum;
					sampler2D->texGLid = DEMO->fboManager.getOpenGLTextureID(fbonum);
				}
			}
			// Is it s a normal texture...
			else {
				sampler2D->texture = DEMO->textureManager.addTexture(s_demo + string_value);
				sampler2D->texGLid = DEMO->textureManager.getOpenGLTextureID(sampler2D->texture);
			}
		}
	}

	// Unbind any shader used
	glUseProgram(0);
}

void sGLSLShaderBind::init() {
	
}

void sGLSLShaderBind::exec() {
	
	/*Shader *my_shad = DEMO->shaderManager.shader[shader];
	Model *my_model = DEMO->modelManager.model[model];

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	my_shad->use();
	// view/projection transformations
	float zoom = DEMO->camera->Zoom;
	glm::mat4 projection = glm::perspective(glm::radians(zoom), (float)GLDRV->width / (float)GLDRV->height, 0.1f, 100.0f);
	glm::mat4 view = DEMO->camera->GetViewMatrix();
	my_shad->setValue("projection", projection);
	my_shad->setValue("view", view);


	// render the loaded model
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f)); // translate it down so it's at the center of the scene
	model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
	model = glm::rotate(model, DEMO->runTime * glm::radians(-90.0f), glm::vec3(0, 1, 0));
	my_shad->setValue("model", model);

	//my_model->Draw(*my_shad);

	//DEMO->text->RenderText("hola", 0, -1, 1, glm::vec3(1, 0, 0));
	//DEMO->text->RenderText("hola", sin(DEMO->runTime), cos(DEMO->runTime), sin(DEMO->runTime), glm::vec3(1, 0, 0));
	//glEnable(GL_CULL_FACE);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//DEMO->text->RenderText("hola New", -0.5f*cos(DEMO->runTime), -0.5f*sin(DEMO->runTime), 0.4f, glm::vec3(1, 0, 0));
	//DEMO->text->RenderText("AbraCadraBRA", -0.8f, 0, 0.4f, glm::vec3(1, 0, 0));
	*/
}

void sGLSLShaderBind::end() {
	
}
