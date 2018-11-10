// modelnmanager.cpp
// Spontz Demogroup

#include "main.h"
#include "modelmanager.h"


// Init vars
ModelManager::ModelManager() {
	
}

void ModelManager::init() {
	LOG->Info(LOG_MED, "ModelManager Inited. Clearing internal lists...");
	model.clear();
}

// Adds a Model into the queue, returns the ID of the model added
int ModelManager::addModel(string path) {
	int i;
	int model_id = -1;

	// check if Model is already loaded, then we just return the ID of our Model
	for (i = 0; i < model.size(); i++) {
		if (model[i]->filename.compare(path) == 0) {
			return i;
		}
	}

	// if we must load the model...
	Model *new_model = NULL;
	new_model = new Model(path);
	if (new_model) {
		model.push_back(new_model);
		model_id = (int)model.size() - 1;
		LOG->Info(LOG_MED, "Model %s [id: %d] loaded OK", path.c_str(), model_id);
	}
	else
		LOG->Error("Could not load model: %s", path.c_str());
	
	return model_id;
}


