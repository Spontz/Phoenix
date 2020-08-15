#include "main.h"

struct sLight : public Section {
public:
	sLight();
	bool		load();
	void		init();
	void		exec();
	void		end();
	std::string debug();

private:
	int			lightNum;			// Light Number
	int			linkPostoCamera;	// Link the light to the Camera Position
	int			shadowMapping;		// Is the light being used for shadowMapping?
	float		near_plane;			// shadowMapping: Near plane used in Orthographic view
	float		far_plane;			// shadowMapping: Far plane used in Orthographic view
	float		size;				// shadowMapping: Size of the plane used in Orthographic view
	int			draw;				// Draw a cube representing the light: usefult for debugging
	float		draw_size;			// Size of our debug cube
	mathDriver	*exprLight;			// A equation containing the calculations of the light

};

// ******************************************************************

Section* instance_light() {
	return new sLight();
}

sLight::sLight() {
	type = SectionType::LightSec;
}

bool sLight::load() {
	// script validation
	if ((param.size() != 8)) {
		LOG->Error("Light [%s]: 8 params needed (light Number, link to camera position, shadowMapping, near&far planes, size, DebugDraw & DebugDraw size)", identifier.c_str());
		return false;
	}

	// Load the parameters
	lightNum = (int)param[0];
	if (lightNum<0 || lightNum >= m_demo.lightManager.light.size()) {
		LOG->Error("Light: The light number is not supported by the engine. Max Lights: %d", (m_demo.lightManager.light.size()-1));
		return false;
	}

	linkPostoCamera = (int)param[1];
	
	// Load the parameters for shadow mapping
	shadowMapping = (int)param[2];
	near_plane = param[3];
	far_plane = param[4];
	size = param[5];
	draw = (int)param[6];
	draw_size = param[7];

	Light* my_light = m_demo.lightManager.light[lightNum];

	// Register the variables
	exprLight = new mathDriver(this);
	std::string expr;
	for (int i = 0; i < strings.size(); i++)
		expr += strings[i];
	expr = Util::replaceString(expr, "light_", "light" + std::to_string(lightNum) + "_");	// Adds the name of the light that we want to modify
	exprLight->expression = expr;															// Loads the expression, properly composed
	exprLight->Expression.register_symbol_table(exprLight->SymbolTable);
	if (!exprLight->compileFormula())
		return false;
	return true;
}

void sLight::init() {
}

void sLight::exec() {
	// Evaluate the expression
	exprLight->Expression.value();

	Light* my_light = m_demo.lightManager.light[lightNum];

	if (linkPostoCamera) {
		my_light->position = m_demo.camera->Position;
		my_light->direction = m_demo.camera->Position + (m_demo.camera->Front*10.0f); // TODO: Remove this hardcode! XD
	}
		

	if (shadowMapping){
		// Calculate the Space Matrix of our camera for being used in other sections
		my_light->CalcSpaceMatrix(-(size), size, -(size), size, near_plane, far_plane);
	}

	if (draw) {
		my_light->draw(draw_size);
	}		
}

void sLight::end() {
}

std::string sLight::debug() {
	std::string msg;
	msg = "[ lightsection id: " + identifier + " layer:" + std::to_string(layer) + " ]\n";
	msg += " light: " + std::to_string(lightNum) + "\n";
	return msg;
}
