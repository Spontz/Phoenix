#include "main.h"
#include "core/particleMesh.h"
#include "core/drivers/mathdriver.h"
#include "core/shadervars.h"

struct sDrawParticlesScene : public Section {
public:
	sDrawParticlesScene();
	bool		load();
	void		init();
	void		exec();
	void		end();
	std::string debug();

private:
	// 3D Model
	int				model;

	// Particle engine variables
	int				numParticles;
	ParticleMesh*	pSystem;
	Shader*			shader;

	// Particle Matrix positioning (for all the model)
	glm::vec3		translation;
	glm::vec3		rotation;
	glm::vec3		scale;

	mathDriver		*exprPosition;	// A equation containing the calculations to position the object
	ShaderVars		*vars;			// For storing any other shader variables
};

// ******************************************************************

Section* instance_drawParticlesScene() {
	return new sDrawParticlesScene();
}

sDrawParticlesScene::sDrawParticlesScene() {
	type = SectionType::DrawParticles;
}

bool sDrawParticlesScene::load() {
	// script validation
	if (strings.size() != 5) {
		LOG->Error("Draw Particles Scene [%s]: 5 strings needed (1 for shader files, 1 for 3D model, 3 for positioning)", identifier.c_str());
		return false;
	}

	// Load the shader
	shader = m_demo.shaderManager.addShader(m_demo.dataFolder + strings[0]);
	if (!shader)
		return false;

	// Load the model scene
	model = m_demo.modelManager.addModel(m_demo.dataFolder + strings[1]);

	if (model <0)
		return false;

	// Load model properties
	Model* my_model;
	my_model = m_demo.modelManager.model[model];

	// Calculate particles number
	numParticles = 0;
	for (int i = 0; i < my_model->meshes.size(); i++) {
		numParticles += (int)my_model->meshes[i].unique_vertices_pos.size();
	}
	if (numParticles == 0) {
		LOG->Error("Draw Particles Scene [%s]: No vertex found in the model", identifier.c_str());
		return false;
	}
	// Load the particles position
	std::vector<glm::vec3> Pos;
	Pos.resize(numParticles);
	int cnt = 0;
	for (int i = 0; i < my_model->meshes.size(); i++) {
		for (int j = 0; j < my_model->meshes[i].unique_vertices_pos.size(); j++) {
			Pos[cnt] = my_model->meshes[i].unique_vertices_pos[j];
			cnt++;
		}
	}



	// Load particle positioning
	exprPosition = new mathDriver(this);
	// Load all the other strings
	for (int i = 2; i < strings.size(); i++)
		exprPosition->expression += strings[i];

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

	// Create the particle system
	pSystem = new ParticleMesh(numParticles);
	if (!pSystem->startup(Pos))
		return false;
	// Delete all temporarly elements
	Pos.clear();

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

void sDrawParticlesScene::init() {
}


static float lastTime = 0;

void sDrawParticlesScene::exec() {
	// Start evaluating blending
	EvalBlendingStart();

	// Evaluate the expression
	exprPosition->Expression.value();

	glDepthMask(GL_FALSE); // Disable depth buffer writting

	glm::mat4 projection = glm::perspective(glm::radians(m_demo.camera->Zoom), GLDRV->GetCurrentViewport().GetAspectRatio(), 0.1f, 10000.0f);
	glm::mat4 view = m_demo.camera->GetViewMatrix();

	// render the loaded model
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, translation);
	model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0));
	model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0, 0, 1));
	model = glm::scale(model, scale);

	
	// Get the shader
	shader->use();
	shader->setValue("gTime", runTime);	// Send the Time
	shader->setValue("gVP", projection * view);	// Set Projection x View matrix
	shader->setValue("gModel", model);			// Set Model matrix
	shader->setValue("gCameraPos", m_demo.camera->Position);		// Set camera position
	shader->setValue("gNumParticles", (float)numParticles);	// Set the total number of particles

	// Set the other shader variable values
	vars->setValues();

	// Render particles
	pSystem->render(runTime);

	// End evaluating blending
	glDepthMask(GL_TRUE); // Enable depth buffer writting
	EvalBlendingEnd();

}

void sDrawParticlesScene::end() {
	pSystem->shutdown();
}

std::string sDrawParticlesScene::debug() {
	std::string msg;
	msg += "[ drawParticlesScene id: " + identifier + " layer:" + std::to_string(layer) + " ]\n";
	msg += " numParticles: " + std::to_string(numParticles) + "\n";
	return msg;
}
