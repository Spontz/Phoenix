// ModelManager.cpp
// Spontz Demogroup

#include "main.h"
#include "core/renderer/ModelManager.h"

namespace Phoenix {

	// Adds a Model into the queue, returns the ID of the model added
	SP_Model ModelManager::addModel(std::string_view path) {
		SP_Model p_model;

		// check if Model is already loaded, then we just return the ID of our Model
		for (auto i = 0; i < model.size(); i++)
			if (model[i]->filepath.compare(path) == 0)
				return model[i];

		// if we must load the model...
		SP_Model new_model = std::make_shared<Model>();
		if (new_model->Load(path)) {
			model.emplace_back(new_model);
			p_model = new_model;
			Logger::info(LogLevel::med, "Model {} [id: {}] loaded OK", path, model.size() - 1);
		}
		else
			Logger::error("Could not load model: {}", path);

		return p_model;
	}

	void ModelManager::clear()
	{
		model.clear();
	}
}