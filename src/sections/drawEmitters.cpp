#include "main.h"
#include "core/particleSystem.h"
#include "core/shadervars.h"

struct sDrawEmitters : public Section {
public:
	sDrawEmitters();
	bool		load();
	void		init();
	void		exec();
	void		end();
	std::string debug();

private:
	// 3D Model
	int			model;

	// Particle engine variables
	unsigned int	numMaxParticles;
	unsigned int	numEmitters;
	float			currentEmitter;
	float			emissionTime;
	float			particleLifeTime;
	float			particleSpeed;
	float			emitterRandomness;
	ParticleSystem *pSystem;

	// Particles positioning (for all the model)
	glm::vec3	translation;
	glm::vec3	rotation;
	glm::vec3	scale;

	glm::vec3	velocity;
	glm::vec3	force;
	glm::vec3	color;
	mathDriver	*exprPosition;	// A equation containing the calculations to position the object

};

Section* instance_drawEmitters() {
	return new sDrawEmitters();
}

sDrawEmitters::sDrawEmitters() {
	type = SectionType::DrawEmitters;
}

static float RandomFloat()
{
	float Max = RAND_MAX;
	return ((float)rand() / Max);
}

bool sDrawEmitters::load() {
	// script validation
	if ((param.size() != 3) || (strings.size() != 9)) {
		LOG->Error("Draw Emitters [%s]: 3 param (emission time, Particle Life Time & Randomness) and 9 strings needed (shader path, model, 3 for positioning, part speed, velocity, force and color)", identifier.c_str());
		return false;
	}

	// Load the shaders
	std::string pathShaders;
	pathShaders = m_demo.dataFolder + strings[0];

	// Load the model scene
	model = m_demo.modelManager.addModel(m_demo.dataFolder + strings[1]);

	if (model < 0)
		return false;

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

	exprPosition->SymbolTable.add_variable("partSpeed", particleSpeed);
	exprPosition->SymbolTable.add_variable("velX", velocity.x);
	exprPosition->SymbolTable.add_variable("velY", velocity.y);
	exprPosition->SymbolTable.add_variable("velZ", velocity.z);

	exprPosition->SymbolTable.add_variable("forceX", force.x);
	exprPosition->SymbolTable.add_variable("forceY", force.y);
	exprPosition->SymbolTable.add_variable("forceZ", force.z);

	exprPosition->SymbolTable.add_variable("colorR", color.r);
	exprPosition->SymbolTable.add_variable("colorG", color.g);
	exprPosition->SymbolTable.add_variable("colorB", color.b);

	exprPosition->SymbolTable.add_variable("nE", currentEmitter);

	// Load model properties
	Model *my_model;
	my_model = m_demo.modelManager.model[model];

	// Load the particle generator parameters
	int numEmitters = 0;
	for (int i = 0; i < my_model->meshes.size(); i++) {
		numEmitters += (int)my_model->meshes[i].unique_vertices_pos.size();
	}

	numEmitters = numEmitters;
	if (numEmitters <= 0) {
		LOG->Error("Draw Emitters [%s]: No emitters found in the 3D model", identifier.c_str());
		return false;
	}

	exprPosition->SymbolTable.add_constant("TnE", (float)numEmitters);
	
	exprPosition->Expression.register_symbol_table(exprPosition->SymbolTable);
	if (!exprPosition->compileFormula())
		return false;

	emissionTime = param[0];
	if (emissionTime <= 0) {
		LOG->Error("Draw Emitters [%s]: Emission time should be greater than 0", identifier.c_str());
		return false;
	}

	//TODO: Hack to remove
	numEmitters = 1;

	particleLifeTime = param[1];
	numMaxParticles = numEmitters + static_cast<unsigned int>(static_cast<float>(numEmitters)*particleLifeTime*(1.0f / emissionTime));
	
	// Emitter Ramdomness
	emitterRandomness = param[2];

	LOG->Info(LogLevel::LOW, "Draw Emitters [%s]: Num max of particles will be: %d", identifier.c_str(), numMaxParticles);

	std::vector<Particle> Emitter;
	Emitter.resize(numEmitters);

	currentEmitter = 0;
	Emitter[0].Type = PARTICLE_TYPE_EMITTER;
	Emitter[0].Pos = glm::vec3(0 ,0 ,0);
	Emitter[0].Vel = glm::vec3(0, 0, 0);
	Emitter[0].Col = glm::vec3(1, 0, 0);
	Emitter[0].lifeTime = 0.0f;
	/*
	// Load the emitters, based in our model vertexes
	int numEmitter = 0;
	currentEmitter = 0;
	for (int i = 0; i < my_model->meshes.size(); i++) {
		for (int j = 0; j < my_model->meshes[i].unique_vertices_pos.size(); j++) {
			exprPosition->Expression.value(); // Evaluate the expression on each particle, just in case something has changed
			Emitter[numEmitter].Type = PARTICLE_TYPE_EMITTER;
			Emitter[numEmitter].Pos = my_model->meshes[i].unique_vertices_pos[j];
			Emitter[numEmitter].Vel = velocity + (emitterRandomness * glm::vec3(RandomFloat(), RandomFloat(), RandomFloat()));
			Emitter[numEmitter].Col = color;
			Emitter[numEmitter].lifeTime = 0.0f;
			numEmitter++;
			currentEmitter = static_cast<float>(numEmitter);
		}
	}
	*/
	// Create the particle system
	pSystem = new ParticleSystem(pathShaders, numMaxParticles, numEmitters, emissionTime, particleLifeTime);
	if (!pSystem->InitParticleSystem(this, Emitter, uniform))
		return false;

	Emitter.clear();
	return true;
}

void sDrawEmitters::init() {
}


static float lastTime = 0;

void sDrawEmitters::exec() {
	
	// Start evaluating blending
	EvalBlendingStart();

	// Evaluate the expression
	exprPosition->Expression.value();
	
	glDepthMask(GL_FALSE); // Disable depth buffer writting

	glm::mat4 projection = glm::perspective(glm::radians(DEMO->camera->Zoom), GLDRV->GetCurrentViewport().GetAspectRatio(), 0.1f, 10000.0f);
	glm::mat4 view = m_demo.camera->GetViewMatrix();

	// render the loaded model
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, translation);
	model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0));
	model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0, 0, 1));
	model = glm::scale(model, scale);
	
	glm::mat4 vp = projection * view;	//TODO: This mutliplication should be done in the shader, by passing the 2 matrix
	
	// Render particles
	float deltaTime = runTime - lastTime;
	deltaTime = deltaTime * particleSpeed;
	lastTime = runTime;
	if (deltaTime < 0) {
		deltaTime = -deltaTime;	// In case we rewind the demo
	}
	pSystem->force = force;
	
	pSystem->Render(deltaTime, vp, model, m_demo.camera->Position);


	// End evaluating blending
	glDepthMask(GL_TRUE); // Enable depth buffer writting
	EvalBlendingEnd();

}

void sDrawEmitters::end() {
}

std::string sDrawEmitters::debug() {
	std::string msg;
	msg += "[ drawEmitters id: " + identifier + " layer:" + std::to_string(layer) + " ]\n";
	msg += " numEmitters: " + std::to_string(numEmitters) + "\n";
	msg += " numMaxParticles: " + std::to_string(numMaxParticles) + "\n";
	return msg;
}
