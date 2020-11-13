// ModelManager.h
// Spontz Demogroup

#pragma once

#include <string>
#include <vector>
#include "core/renderer/Model.h"


class ModelManager {
public:
	std::vector<Model*> model;	// Model list

	ModelManager();
	virtual ~ModelManager();
	Model* addModel(std::string path);
private:


	
};