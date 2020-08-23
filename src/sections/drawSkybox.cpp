#include "main.h"
#include "core/drivers/mathdriver.h"
#include "core/shadervars.h"

struct sDrawSkybox : public Section {
public:
	sDrawSkybox();
	bool		load();
	void		init();
	void		exec();
	void		end();
	std::string debug();

private:
	Cubemap*	cubemap;
	int			model;
	int			shader;
	int			enableDepthBufferClearing;
	int			drawWireframe;

	glm::vec3	rotation;
	glm::vec3	scale;
	
	mathDriver	*exprPosition;	// A equation containing the calculations to position the object
};

// ******************************************************************

Section* instance_drawSkybox() {
	return new sDrawSkybox();
}

sDrawSkybox::sDrawSkybox() {
	type = SectionType::DrawSkybox;
}

bool sDrawSkybox::load() {
	if ((param.size() != 2) || (strings.size() != 8)) {
		LOG->Error("DrawSkybox [%s]: 2 param and 8 strings needed: enable depthBuffer, drawWireframe + 6 strings with skybox faces, 2 strings with rot and scale", identifier.c_str());
		return false;
	}

	// Check if skybox is present in resources
	if (m_demo.res->obj_skybox == 0 || m_demo.res->shdr_Skybox == nullptr ) {
		LOG->Error("DrawSkybox [%s]: Skybox model or shader has not been properly loaded in resources, please fix it!", identifier.c_str());
		return false;
	}

	// Depth Buffer Clearing Flag
	enableDepthBufferClearing = (int)param[0];
	drawWireframe = (int)param[1];
	
	// Load the 6 textures of our cubemap
	std::vector<std::string> faces {	m_demo.dataFolder + strings[0], m_demo.dataFolder + strings[1], m_demo.dataFolder + strings[2],
										m_demo.dataFolder + strings[3], m_demo.dataFolder + strings[4], m_demo.dataFolder + strings[5]};

	cubemap = m_demo.textureManager.addCubemap(faces, false);
	if (!cubemap)
		return false;

	// Read variables for traslation, rotation and scaling
	exprPosition = new mathDriver(this);
	// Load positions, process constants and compile expression
	exprPosition->expression = std::string(strings[6]) + strings[7]; // Concatenate the 2 positioning strings (rotation+scale)
	exprPosition->SymbolTable.add_variable("rx", rotation.x);
	exprPosition->SymbolTable.add_variable("ry", rotation.y);
	exprPosition->SymbolTable.add_variable("rz", rotation.z);
	exprPosition->SymbolTable.add_variable("sx", scale.x);
	exprPosition->SymbolTable.add_variable("sy", scale.y);
	exprPosition->SymbolTable.add_variable("sz", scale.z);
	exprPosition->Expression.register_symbol_table(exprPosition->SymbolTable);
	if (!exprPosition->compileFormula())
		return false;

	return true;
}

void sDrawSkybox::init() {
	
}

void sDrawSkybox::exec() {
	// Evaluate the expression
	exprPosition->Expression.value();

	// Start evaluating blending
	EvalBlendingStart();
	
	// change depth function so depth test passes when values are equal to depth buffer's content
	glDepthFunc(GL_LEQUAL);


	if (drawWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (enableDepthBufferClearing)
		glClear(GL_DEPTH_BUFFER_BIT);

	m_demo.res->shdr_Skybox->use(); // TODO: Do not use the Resource shader for skybox, and use our own shader!

	// view/projection transformations
	float zoom = m_demo.camera->Zoom;
	glm::mat4 projection = glm::perspective(glm::radians(zoom), GLDRV->GetCurrentViewport().GetAspectRatio(), 0.1f, 10000.0f);
	glm::mat4 view = glm::mat4(glm::mat3(m_demo.camera->GetViewMatrix())); // remove translation from the view matrix
	m_demo.res->shdr_Skybox->setValue("projection", projection);
	m_demo.res->shdr_Skybox->setValue("view", view);

	// render the loaded model
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0));
	model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0, 0, 1));
	model = glm::scale(model, scale);
	m_demo.res->shdr_Skybox->setValue("model", model);
	
	m_demo.res->shdr_Skybox->setValue("skybox", 0);
	m_demo.res->Draw_Skybox(cubemap);
	

	if (drawWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	// End evaluating blending
	EvalBlendingEnd();
}

void sDrawSkybox::end() {
	
}

std::string sDrawSkybox::debug() {
	return "[ drawSkybox id: " + identifier + " layer:" + std::to_string(layer) + " ]\n";
}
