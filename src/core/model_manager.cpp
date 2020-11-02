// model_manager.cpp
// Spontz Demogroup

#include "main.h"
#include "model_manager.h"


// Init vars
ModelManager::ModelManager() {
	model.clear();
}

ModelManager::~ModelManager()
{
	model.clear();
}

// Adds a Model into the queue, returns the ID of the model added
Model* ModelManager::addModel(std::string path) {
	int i;
	Model* p_model = nullptr;

	// check if Model is already loaded, then we just return the ID of our Model
	for (i = 0; i < model.size(); i++) {
		if (model[i]->filepath.compare(path) == 0) {
			return model[i];
		}
	}

	// if we must load the model...
	Model *new_model = new Model();
	if (new_model->Load(path)) {
		model.push_back(new_model);
		p_model = new_model;
		LOG->Info(LogLevel::MED, "Model %s [id: %d] loaded OK", path.c_str(), model.size()-1);
	}
	else
		LOG->Error("Could not load model: %s", path.c_str());
	
	return p_model;
}


