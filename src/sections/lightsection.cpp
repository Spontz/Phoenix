#include "main.h"

typedef struct {
	int			lightNum;			// Light Number
	int			linkPostoCamera;	// Link the light to the Camera Position
	int			shadowMapping;		// Is the light being used for shadowMapping?
	float		near_plane;			// shadowMapping: Near plane used in Orthographic view
	float		far_plane;			// shadowMapping: Far plane used in Orthographic view
	float		size;				// shadowMapping: Size of the plane used in Orthographic view
	int			draw;				// Draw a cube representing the light: usefult for debugging
	mathDriver	*exprLight;			// A equation containing the calculations of the light

} light_section;

static light_section *local;

sLight::sLight() {
	type = SectionType::LightSec;
}



bool sLight::load() {
	// script validation
	if ((this->param.size() != 7)) {
		LOG->Error("Light [%s]: 7 params needed (light Number, link to camera position, shadowMapping, near&far planes, size and DebugDraw)", this->identifier.c_str());
		return false;
	}

	local = (light_section*)malloc(sizeof(light_section));
	this->vars = (void *)local;

	// Load the parameters
	local->lightNum = (int)this->param[0];
	if (local->lightNum<0 || local->lightNum >= DEMO->lightManager.light.size()) {
		LOG->Error("Light: The light number is not supported by the engine. Max Lights: %d", (DEMO->lightManager.light.size()-1));
		return false;
	}

	local->linkPostoCamera = (int)this->param[1];
	
	// Load the parameters for shadow mapping
	local->shadowMapping = (int)this->param[2];
	local->near_plane = this->param[3];
	local->far_plane = this->param[4];
	local->size = this->param[5];
	local->draw = (int)this->param[6];

	Light* my_light = DEMO->lightManager.light[local->lightNum];

	// Register the variables
	local->exprLight = new mathDriver(this);
	string expr;
	for (int i = 0; i < this->strings.size(); i++)
		expr += this->strings[i];
	expr = Util::replaceString(expr, "light_", "light" + std::to_string(local->lightNum) + "_");	// Adds the name of the light that we want to modify
	local->exprLight->expression = expr;															// Loads the expression, properly composed
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

	Light* my_light = DEMO->lightManager.light[local->lightNum];

	if (local->linkPostoCamera) {
		my_light->position = DEMO->camera->Position;
		my_light->direction = DEMO->camera->Position + (DEMO->camera->Front*10.0f); // TODO: Remove this hardcode! XD
	}
		

	if (local->shadowMapping){
		// Calculate the Space Matrix of our camera for being used in other sections
		my_light->CalcSpaceMatrix(-(local->size), local->size, -(local->size), local->size, local->near_plane, local->far_plane);
	}

	if (local->draw) {
		my_light->draw(0.1f);
	}		
}

void sLight::end() {
}

string sLight::debug() {
	local = (light_section *)this->vars;

	string msg;
	msg = "[ lightsection id: " + this->identifier + " layer:" + std::to_string(this->layer) + " ]\n";
	msg += " light: " + std::to_string(local->lightNum) + "\n";
	return msg;
}
