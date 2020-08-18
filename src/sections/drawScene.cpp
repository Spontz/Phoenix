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
	int			model;
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
	
	// Previous model, projection and view matrix, for being used in effects like motion blur
	glm::mat4	prev_model;
	glm::mat4	prev_projection;
	glm::mat4	prev_view;

	mathDriver	*exprPosition;	// A equation containing the calculations to position the object
	ShaderVars	*vars;			// For storing any other shader variables
};

// ******************************************************************

Section* instance_drawScene() {
	return new sDrawScene();
}

sDrawScene::sDrawScene() {
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
	if (model < 0)
		return false;
	shader = m_demo.shaderManager.addShader(m_demo.dataFolder + this->strings[1]);
	if (!shader)
		return false;
	

	// Load model properties
	Model *my_model;
	my_model = m_demo.modelManager.model[model];
	my_model->playAnimation = playAnimation;
	if (my_model->playAnimation)
		my_model->setAnimation(AnimationNumber);

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
		my_model->useCamera = false;
	else
		my_model->setCamera((unsigned int)CameraNumber);


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
	
	Model *my_model = m_demo.modelManager.model[model];
	
	// Start evaluating blending
	EvalBlendingStart();

	// Evaluate the expression
	exprPosition->Expression.value();

	if (drawWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (enableDepthBufferClearing == 1)
		glClear(GL_DEPTH_BUFFER_BIT);

	// Set model properties
	my_model->playAnimation = playAnimation;
	if (my_model->playAnimation)
		my_model->setAnimation(AnimationNumber);
	if (CameraNumber < 0)
		my_model->useCamera = false;
	else
		my_model->setCamera((unsigned int)CameraNumber);

	// Load shader
	shader->use();

	// For ShadowMapping
	shader->setValue("lightSpaceMatrix", m_demo.lightManager.light[0]->spaceMatrix);
	// End ShadowMapping

	// view/projection transformations
	glm::mat4 projection = glm::perspective(
		glm::radians(m_demo.camera->Zoom),
		GLDRV->GetFramebufferViewport().GetAspectRatio(),
		0.1f, 10000.0f
	);

	shader->setValue("projection", projection);

	glm::mat4 view = m_demo.camera->GetViewMatrix();
	//if (CameraNumber < 0)
		shader->setValue("view", view);

	


	// render the loaded model
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, translation);
	model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0));
	model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0, 0, 1));
	model = glm::scale(model, scale);
	my_model->modelTransform = model;

	// For MotionBlur
	shader->setValue("prev_projection", prev_projection);
	//if (CameraNumber < 0)
		shader->setValue("prev_view", prev_view);
	shader->setValue("prev_model", prev_model);

	prev_projection = projection;
	//if (CameraNumber < 0)
		prev_view = view;
	prev_model = model;
	// End MotionBlur

	// Set the other shader variable values
	vars->setValues();

	my_model->Draw(shader->ID, AnimationTime);

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
	Model* my_model = m_demo.modelManager.model[model];

	std::string msg;
	msg = "[ drawScene id: " + this-> identifier + " layer:" + std::to_string(this->layer) + " ]\n";
	msg += " file: " + my_model->filename + "\n";
	msg += " meshes: " + std::to_string(my_model->meshes.size()) + "\n";
/*	for (int i=0; i<my_model->meshes.size(); i++) {
		msg += "  mesh: " + std::to_string(i) + "\n";
		msg += "    Num vertices: " + std::to_string(my_model->meshes[i].vertices.size()) + "\n";
		msg += "    Num textures: " + std::to_string(my_model->meshes[i].material.textures.size()) + "\n";
		msg += "    Color Diffuse [" +	std::to_string(my_model->meshes[i].material.colDiffuse.r) + " / " +
										std::to_string(my_model->meshes[i].material.colDiffuse.g) + " / " +
										std::to_string(my_model->meshes[i].material.colDiffuse.b) + " ] " + "\n";
		msg += "    Color Ambient [" + std::to_string(my_model->meshes[i].material.colAmbient.r) + " / " +
										std::to_string(my_model->meshes[i].material.colAmbient.g) + " / " +
										std::to_string(my_model->meshes[i].material.colAmbient.b) + " ] " + "\n";
		msg += "    Color Specular [" + std::to_string(my_model->meshes[i].material.colSpecular.r) + " / " +
										std::to_string(my_model->meshes[i].material.colSpecular.g) + " / " +
										std::to_string(my_model->meshes[i].material.colSpecular.b) + " ] " + "\n";
	}
*/	
	return msg;
}
