// modelnmanager.h
// Spontz Demogroup

#ifndef MODELMANAGER_H
#define MODELMANAGER_H
#include <string>
#include <vector>
#include "core/model.h"


class ModelManager {
public:
	std::vector<Model*> model;	// Model list

	ModelManager();
	virtual ~ModelManager();
	int addModel(std::string path);
private:


	
};

#endif