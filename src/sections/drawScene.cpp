#include "main.h"
#include "core/drivers/mathdriver.h"
#include "core/shadervars.h"

struct sDrawScene : public Section {
public:
	sDrawScene();
	bool		load();
	void		init();
	void		exec();
	void		end();
	std::string debug();

private:
	Model*		model;
	Shader*		shader;
	int			enableDepthBufferClearing;
	int			drawWireframe;
	float		CameraNumber;		// Number of the camera to use (-1 = means to not use camera)
	bool		playAnimation;		// Do we want to play the animation?
	int			AnimationNumber;	// Number of animation to play
	float		AnimationTime;		// Animation time (in seconds)

	glm::vec3	translation;
	glm::vec3	rotation;
	glm::vec3	scale;

	// Model, projection and view matrix
	glm::mat4	mat_model;
	glm::mat4	mat_projection;
	glm::mat4	mat_view;

	// Previous model, projection and view matrix, for being used in effects like motion blur
	glm::mat4	mat_prev_model;
	glm::mat4	mat_prev_projection;
	glm::mat4	mat_prev_view;

	mathDriver	*exprPosition;	// A equation containing the calculations to position the object
	ShaderVars	*vars;			// For storing any other shader variables
};

// ******************************************************************

Section* instance_drawScene() {
	return new sDrawScene();
}

sDrawScene::sDrawScene() 
:
	model (nullptr),
	shader(nullptr),
	enableDepthBufferClearing(1),
	drawWireframe(0),
	CameraNumber(-1),
	playAnimation(0),
	AnimationNumber(0),
	AnimationTime(0),
	translation({ 0,0,0 }),
	rotation({ 0,0,0 }),
	scale({ 1,1,1 }),
	mat_model(glm::mat4(1.0f)),
	mat_projection(glm::mat4(1.0f)),
	mat_view(glm::mat4(1.0f)),
	mat_prev_model(glm::mat4(1.0f)),
	mat_prev_projection(glm::mat4(1.0f)),
	mat_prev_view(glm::mat4(1.0f)),
	exprPosition(nullptr),
	vars(nullptr)
{
	type = SectionType::DrawScene;
}

bool sDrawScene::load() {
	if ((this->param.size() != 4) || (this->strings.size() < 7)) {
		LOG->Error("DrawScene [%s]: 4 param (Enable Depth buffer, enable wireframe, enable animation and animation number) and 7 strings needed (model, shader, CameraNumber, Time and 3 for object positioning)", this->identifier.c_str());
		return false;
	}



	// Load default values
	AnimationNumber = 0;
	CameraNumber = -1;
	playAnimation = false;
	AnimationTime = 0;

	// Depth Buffer Clearing Flag
	enableDepthBufferClearing = (int)this->param[0];
	drawWireframe= (int)this->param[1];

	// Animation parameters
	playAnimation = (int)this->param[2];
	AnimationNumber = (int)this->param[3];
	
	// Load model and shader
	model = m_demo.modelManager.addModel(m_demo.dataFolder + this->strings[0]);
	if (!model)
		return false;
	shader = m_demo.shaderManager.addShader(m_demo.dataFolder + this->strings[1]);
	if (!shader)
		return false;
	
	// Load model properties
	model->playAnimation = playAnimation;
	if (model->playAnimation)
		model->setAnimation(AnimationNumber);

	exprPosition = new mathDriver(this);
	// Load all the other strings
	for (int i = 2; i < strings.size(); i++)
		exprPosition->expression += this->strings[i];

	exprPosition->SymbolTable.add_variable("CameraNumber", CameraNumber);
	exprPosition->SymbolTable.add_variable("aTime", AnimationTime);
	exprPosition->SymbolTable.add_variable("tx", translation.x);
	exprPosition->SymbolTable.add_variable("ty", translation.y);
	exprPosition->SymbolTable.add_variable("tz", translation.z);
	exprPosition->SymbolTable.add_variable("rx", rotation.x);
	exprPosition->SymbolTable.add_variable("ry", rotation.y);
	exprPosition->SymbolTable.add_variable("rz", rotation.z);
	exprPosition->SymbolTable.add_variable("sx", scale.x);
	exprPosition->SymbolTable.add_variable("sy", scale.y);
	exprPosition->SymbolTable.add_variable("sz", scale.z);
	exprPosition->Expression.register_symbol_table(exprPosition->SymbolTable);
	if (!exprPosition->compileFormula())
		return false;
	exprPosition->Expression.value();
	// Set the camera number
	if (CameraNumber < 0)
		model->useCamera = false;
	else
		model->setCamera((unsigned int)CameraNumber);


	// Create Shader variables
	shader->use();
	vars = new ShaderVars(this, shader);

	// Read the shader variables
	for (int i = 0; i < this->uniform.size(); i++) {
		vars->ReadString(this->uniform[i].c_str());
	}

	// Set shader variables values
	vars->setValues();

	return true;
}

void sDrawScene::init() {
	
}

void sDrawScene::exec() {
	
	// Start evaluating blending
	EvalBlendingStart();

	// Evaluate the expression
	exprPosition->Expression.value();

	if (drawWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (enableDepthBufferClearing == 1)
		glClear(GL_DEPTH_BUFFER_BIT);

	// Set model properties
	model->playAnimation = playAnimation;
	if (model->playAnimation)
		model->setAnimation(AnimationNumber);
	if (CameraNumber < 0)
		model->useCamera = false;
	else
		model->setCamera((unsigned int)CameraNumber);

	// Load shader
	shader->use();

	// For ShadowMapping
	shader->setValue("lightSpaceMatrix", m_demo.lightManager.light[0]->spaceMatrix);
	// End ShadowMapping

	// view/projection transformations
	mat_projection = glm::perspective(
		glm::radians(m_demo.camera->Zoom),
		GLDRV->GetFramebufferViewport().GetAspectRatio(),
		0.1f, 10000.0f
	);

	shader->setValue("projection", mat_projection);

	mat_view = m_demo.camera->GetViewMatrix();
	//if (CameraNumber < 0)
		shader->setValue("view", mat_view);


	// render the loaded scene
	mat_model = glm::mat4(1.0f);
	mat_model = glm::translate(mat_model, translation);
	mat_model = glm::rotate(mat_model, glm::radians(rotation.x), glm::vec3(1, 0, 0));
	mat_model = glm::rotate(mat_model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
	mat_model = glm::rotate(mat_model, glm::radians(rotation.z), glm::vec3(0, 0, 1));
	mat_model = glm::scale(mat_model, scale);
	model->modelTransform = mat_model;

	// For MotionBlur
	shader->setValue("prev_projection", mat_prev_projection);
	//if (CameraNumber < 0)
		shader->setValue("prev_view", mat_prev_view);
	shader->setValue("prev_model", mat_prev_model);

	mat_prev_projection = mat_projection;
	//if (CameraNumber < 0)
		mat_prev_view = mat_view;
	mat_prev_model = mat_model;
	// End MotionBlur

	// Set the other shader variable values
	vars->setValues();

	model->Draw(shader->ID, AnimationTime);

	glUseProgram(0);
	if (drawWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	// End evaluating blending
	EvalBlendingEnd();
}

void sDrawScene::end() {
	
}

std::string sDrawScene::debug()
{
	std::string msg;
	msg = "[ drawScene id: " + this-> identifier + " layer:" + std::to_string(this->layer) + " ]\n";
	msg += " file: " + model->filename + "\n";
	msg += " meshes: " + std::to_string(model->meshes.size()) + "\n";
/*	for (int i=0; i<my_model->meshes.size(); i++) {
		msg += "  mesh: " + std::to_string(i) + "\n";
		msg += "    Num vertices: " + std::to_string(model->meshes[i].vertices.size()) + "\n";
		msg += "    Num textures: " + std::to_string(model->meshes[i].material.textures.size()) + "\n";
		msg += "    Color Diffuse [" +	std::to_string(model->meshes[i].material.colDiffuse.r) + " / " +
										std::to_string(model->meshes[i].material.colDiffuse.g) + " / " +
										std::to_string(model->meshes[i].material.colDiffuse.b) + " ] " + "\n";
		msg += "    Color Ambient [" + std::to_string(model->meshes[i].material.colAmbient.r) + " / " +
										std::to_string(model->meshes[i].material.colAmbient.g) + " / " +
										std::to_string(model->meshes[i].material.colAmbient.b) + " ] " + "\n";
		msg += "    Color Specular [" + std::to_string(model->meshes[i].material.colSpecular.r) + " / " +
										std::to_string(model->meshes[i].material.colSpecular.g) + " / " +
										std::to_string(model->meshes[i].material.colSpecular.b) + " ] " + "\n";
	}
*/	
	return msg;
}
