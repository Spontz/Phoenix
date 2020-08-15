#include "main.h"
#include "core/shadervars.h"

struct sDrawSceneMatrix : public Section {
public:
	sDrawSceneMatrix();
	bool		load();
	void		init();
	void		exec();
	void		end();
	std::string debug();

private:
	int			model_ref;	// Reference model to be use to store positions
	int			model;		// Model to draw
	Shader*		shader;
	int			enableDepthBufferClearing;
	int			drawWireframe;
	int			playAnimation;		// Do we want to play the animation?
	int			AnimationNumber;	// Number of animation to play
	float		AnimationTime;		// Animation time (in seconds)

	float		n;			// Object to draw
	glm::vec3	translation;
	glm::vec3	rotation;
	glm::vec3	scale;

	// Previous model, projection and view matrix, for being used in effects like motion blur
	std::vector<glm::mat4>	*prev_model;		// The model needs to be stored on a vector because we need to store the previous model matrix of each object
	glm::mat4				prev_projection;
	glm::mat4				prev_view;

	mathDriver	*exprPosition;	// A equation containing the calculations to position the object
	ShaderVars	*vars;			// For storing any other shader variables
};

// ******************************************************************

Section* instance_drawSceneMatrix() {
	return new sDrawSceneMatrix();
}

sDrawSceneMatrix::sDrawSceneMatrix() {
	type = SectionType::DrawScene;
}

bool sDrawSceneMatrix::load() {
	if ((param.size() != 4) || (strings.size() < 7)) {
		LOG->Error("DrawSceneMatrix [%s]: 4 param (Enable Depth buffer, enable wireframe, enable animation and animation number) and 7 strings needed", identifier.c_str());
		return false;
	}

	// Load default values
	AnimationNumber = 0;
	playAnimation = false;
	AnimationTime = 0;

	// Depth Buffer Clearing Flag
	enableDepthBufferClearing = (int)param[0];
	drawWireframe= (int)param[1];

	// Animation parameters
	playAnimation = (int)param[2];
	AnimationNumber = (int)param[3];
	
	// Load ref. model, model and shader
	model_ref = m_demo.modelManager.addModel(m_demo.dataFolder + strings[0]);
	model = m_demo.modelManager.addModel(m_demo.dataFolder + strings[1]);
	if (model_ref < 0 || model < 0)
		return false;

	shader = m_demo.shaderManager.addShader(m_demo.dataFolder + strings[2]);
	if (!shader)
		return false;

	// Calculate the number of matrices that we need to store
	Model *my_model_ref;
	my_model_ref = m_demo.modelManager.model[model_ref];
	int num_matrices = 0;
	for (int i = 0; i < my_model_ref->meshes.size(); i++)
	{
		num_matrices += (int)my_model_ref->meshes[i].unique_vertices_pos.size();
	}
	prev_model = new std::vector<glm::mat4>;
	prev_model->resize(num_matrices);

	// Load model properties
	Model *my_model;
	my_model = m_demo.modelManager.model[model];
	my_model->playAnimation = playAnimation;
	if (my_model->playAnimation)
		my_model->setAnimation(AnimationNumber);


	exprPosition = new mathDriver(this);
	// Load all the other strings
	for (int i = 3; i < strings.size(); i++)
		exprPosition->expression += strings[i];

	exprPosition->SymbolTable.add_variable("aTime", AnimationTime);
	exprPosition->SymbolTable.add_variable("n", n);
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

	// Create Shader variables
	shader->use();
	vars = new ShaderVars(this, shader);

	// Read the shader variables
	for (int i = 0; i < uniform.size(); i++) {
		vars->ReadString(uniform[i].c_str());
	}

	// Set shader variables values
	vars->setValues();

	return true;
}

void sDrawSceneMatrix::init() {
	
}

void sDrawSceneMatrix::exec() {
	Model *my_model_ref = m_demo.modelManager.model[model_ref];
	Model *my_model = m_demo.modelManager.model[model];
	
	// Start evaluating blending
	EvalBlendingStart();

	if (drawWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (enableDepthBufferClearing == 1)
		glClear(GL_DEPTH_BUFFER_BIT);

	// Set model properties
	my_model->playAnimation = playAnimation;
	if (my_model->playAnimation)
		my_model->setAnimation(AnimationNumber);

	// Load shader
	shader->use();

	// For ShadowMapping
	shader->setValue("lightSpaceMatrix", m_demo.lightManager.light[0]->spaceMatrix);
	// End ShadowMapping

	// view/projection transformations
	glm::mat4 projection = glm::perspective(
		glm::radians(m_demo.camera->Zoom),
		//GLDRV->GetCurrentViewport().GetAspectRatio(),
		GLDRV->GetFramebufferViewport().GetAspectRatio(),
		0.1f, 10000.0f
	);
	glm::mat4 view = m_demo.camera->GetViewMatrix();
	shader->setValue("projection", projection);
	shader->setValue("view", view);
	// For MotionBlur: send the previous matrix
	shader->setValue("prev_projection", prev_projection);
	shader->setValue("prev_view", prev_view);

	// Set the other shader variable values
	vars->setValues();

	// Set the position of the reference model
	glm::mat4 model;
	n = 0;
	int object = 0;
	for (int i = 0; i < my_model_ref->meshes.size(); i++)
	{
		for (int j = 0; j < my_model_ref->meshes[i].unique_vertices_pos.size(); j++)
		{
			// Evaluate the expression
			exprPosition->Expression.value();
			shader->setValue("n", n); // we send also the number of object to the shader

			model = glm::mat4(1.0f);
			model = glm::translate(model, my_model_ref->meshes[i].unique_vertices_pos[j]);

			// Now render the object using the "model_ref" as a model matrix start position
			model = glm::translate(model, translation);
			model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0));
			model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
			model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0, 0, 1));
			model = glm::scale(model, scale);
			my_model->modelTransform = model;

			// For MotionBlur, we send the previous model matrix, and then store it for later use
			shader->setValue("prev_model", prev_model[0][object]);
			prev_model[0][object] = model;

			my_model->Draw(shader->ID, AnimationTime);

			object++; 
			n = (float)object;
		}
	}
	
	// For MotionBlur: store the previous matrix
	prev_projection = projection;
	prev_view = view;

	glUseProgram(0);
	if (drawWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	// End evaluating blending
	EvalBlendingEnd();
}

void sDrawSceneMatrix::end() {
	
}

std::string sDrawSceneMatrix::debug()
{
	Model *my_model_ref = m_demo.modelManager.model[model_ref];
	Model *my_model = m_demo.modelManager.model[model];

	std::string msg;
	msg = "[ drawSceneMatrix id: " +  identifier + " layer:" + std::to_string(layer) + " ]\n";
	msg += " Matrix file: " + my_model_ref->filename + "\n";
	msg += " file: " + my_model->filename + "\n";
	msg += " objects drawn: " + std::to_string((int)(n)) + "\n";
	msg += " meshes in each scene: " + std::to_string(my_model->meshes.size()) + "\n";
	return msg;
}
