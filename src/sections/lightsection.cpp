#include "main.h"

typedef struct {
	int			linkPostoCamera;	// Link the light to the Camera Position
	int			shadowMapping;		// Is the light being used for shadowMapping?
	float		near_plane;
	float		far_plane;
	float		size;
	mathDriver	*exprLight;			// A equation containing the calculations of the light

} light_section;

static light_section *local;

sLight::sLight() {
	type = SectionType::LightSec;
}

bool sLight::load() {
	// script validation
	if ((this->param.size() != 5) || this->strings.size() != 5) {
		LOG->Error("Light [%s]: 5 params needed (link to camera position, shadowMapping, near&far planes and size) and 5 strings needed (ambient strenght, specular strenght, pos, look and color)", this->identifier.c_str());
		return false;
	}

	local = (light_section*)malloc(sizeof(light_section));
	this->vars = (void *)local;

	// Load the parameters
	local->linkPostoCamera = (int)this->param[0];
	
	// Load the parameters for shadow mapping
	local->shadowMapping = (int)this->param[1];
	local->near_plane = this->param[2];
	local->far_plane = this->param[3];
	local->size = this->param[4];


	// Register the variables
	local->exprLight = new mathDriver(this);
	local->exprLight->expression = this->strings[0] + this->strings[1] + this->strings[2] + this->strings[3] + this->strings[4];
	local->exprLight->SymbolTable.add_variable("ambientStrength", DEMO->light->ambientStrength);
	local->exprLight->SymbolTable.add_variable("specularStrength", DEMO->light->specularStrength);
	local->exprLight->SymbolTable.add_variable("posX", DEMO->light->Position[0]);
	local->exprLight->SymbolTable.add_variable("posY", DEMO->light->Position[1]);
	local->exprLight->SymbolTable.add_variable("posZ", DEMO->light->Position[2]);
	local->exprLight->SymbolTable.add_variable("lookAtX", DEMO->light->lookAt[0]);
	local->exprLight->SymbolTable.add_variable("lookAtY", DEMO->light->lookAt[1]);
	local->exprLight->SymbolTable.add_variable("lookAtZ", DEMO->light->lookAt[2]);
	local->exprLight->SymbolTable.add_variable("colR", DEMO->light->color[0]);
	local->exprLight->SymbolTable.add_variable("colG", DEMO->light->color[1]);
	local->exprLight->SymbolTable.add_variable("colB", DEMO->light->color[2]);
	local->exprLight->Expression.register_symbol_table(local->exprLight->SymbolTable);
	local->exprLight->compileFormula();
	return true;
}

void sLight::init() {
}

void sLight::exec() {
	local = (light_section *)this->vars;
	


	// Evaluate the expression
	local->exprLight->Expression.value();

	if (local->linkPostoCamera)
		DEMO->light->Position = DEMO->camera->Position;

	if (local->shadowMapping){
		// Calculate the Space Matrix of our camera for being used in other sections
		DEMO->light->CalcSpaceMatrix(-(local->size), local->size, -(local->size), local->size, local->near_plane, local->far_plane);
	}
}

void sLight::end() {
}
