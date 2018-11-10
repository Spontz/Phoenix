// modelnmanager.h
// Spontz Demogroup

#ifndef MODELMANAGER_H
#define MODELMANAGER_H
#include <string>
#include <vector>
#include "core/model.h"

using namespace std;

class ModelManager {
public:
	std::vector<Model*> model;	// Model list

	ModelManager();
	void init();
	int addModel(string path);
private:


	
};

#endif