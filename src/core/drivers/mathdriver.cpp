// MathDriver.cpp
// Spontz Demogroup
// fastmath library: https://archive.codeplex.com/?p=fastmathparser

#include "main.h"
#include "core/drivers/MathDriver.h"

MathDriver::MathDriver(Section *sec) {
	expression = "";
	
	// Add the variables that will be used by all sections
	SymbolTable.add_variable("t", sec->runTime);
	SymbolTable.add_variable("tend", sec->duration);
	SymbolTable.add_variable("tdemo", DEMO->m_demoRunTime);
	SymbolTable.add_variable("fps", DEMO->m_fps);
	SymbolTable.add_variable("mouseX", GLDRV->m_mouseX);
	SymbolTable.add_variable("mouseY", GLDRV->m_mouseY);

	
	// Multi-purpose variables
	for (int i = 0; i < MULTIPURPOSE_VARS; i++) {
		std::string varNum = "var" + std::to_string(i);
		SymbolTable.add_variable(varNum, DEMO->m_fVar[i]);
	}

	// Track beats
	for (int i = 0; i < MAX_BEATS; i++) {
		std::string beatNum = "beat" + std::to_string(i);
		SymbolTable.add_variable(beatNum, DEMO->m_fBeat[i]);
	}

	// Camera values
	if (DEMO->m_pCamera != NULL) {
		SymbolTable.add_variable("cam_posX", DEMO->m_pCamera->Position.x);
		SymbolTable.add_variable("cam_posY", DEMO->m_pCamera->Position.y);
		SymbolTable.add_variable("cam_posZ", DEMO->m_pCamera->Position.z);

		SymbolTable.add_variable("cam_upX", DEMO->m_pCamera->Up.x);
		SymbolTable.add_variable("cam_upY", DEMO->m_pCamera->Up.y);
		SymbolTable.add_variable("cam_upZ", DEMO->m_pCamera->Up.z);

		SymbolTable.add_variable("cam_yaw", DEMO->m_pCamera->Yaw);
		SymbolTable.add_variable("cam_pitch", DEMO->m_pCamera->Pitch);
		SymbolTable.add_variable("cam_zoom", DEMO->m_pCamera->Zoom);
	}
	else {
		SymbolTable.add_constant("cam_posX", 0);
		SymbolTable.add_constant("cam_posY", 0);
		SymbolTable.add_constant("cam_posZ", 0);

		SymbolTable.add_constant("cam_upX", 0);
		SymbolTable.add_constant("cam_upY", 0);
		SymbolTable.add_constant("cam_upZ", 100);

		SymbolTable.add_constant("cam_yaw", 0);
		SymbolTable.add_constant("cam_pitch", 0);
		SymbolTable.add_constant("cam_zoom", 0);
	}

	// Light values
	for (int i=0; i<DEMO->m_lightManager.light.size(); i++) {
		std::string lightNum = "light" + std::to_string(i);
		SymbolTable.add_variable(lightNum + "_posX", DEMO->m_lightManager.light[i]->position.x);
		SymbolTable.add_variable(lightNum + "_posY", DEMO->m_lightManager.light[i]->position.y);
		SymbolTable.add_variable(lightNum + "_posZ", DEMO->m_lightManager.light[i]->position.z);

		SymbolTable.add_variable(lightNum + "_dirX", DEMO->m_lightManager.light[i]->direction.x);
		SymbolTable.add_variable(lightNum + "_dirY", DEMO->m_lightManager.light[i]->direction.y);
		SymbolTable.add_variable(lightNum + "_dirZ", DEMO->m_lightManager.light[i]->direction.z);

		SymbolTable.add_variable(lightNum + "_colAmbR", DEMO->m_lightManager.light[i]->colAmbient.x);
		SymbolTable.add_variable(lightNum + "_colAmbG", DEMO->m_lightManager.light[i]->colAmbient.y);
		SymbolTable.add_variable(lightNum + "_colAmbB", DEMO->m_lightManager.light[i]->colAmbient.z);

		
		// TODO: Are this vars really needed? Normally we change it directly in the shader...
		SymbolTable.add_variable(lightNum + "_colDifR", DEMO->m_lightManager.light[i]->colDiffuse.x);
		SymbolTable.add_variable(lightNum + "_colDifG", DEMO->m_lightManager.light[i]->colDiffuse.y);
		SymbolTable.add_variable(lightNum + "_colDifB", DEMO->m_lightManager.light[i]->colDiffuse.z);

		SymbolTable.add_variable(lightNum + "_colSpcR", DEMO->m_lightManager.light[i]->colSpecular.x);
		SymbolTable.add_variable(lightNum + "_colSpcG", DEMO->m_lightManager.light[i]->colSpecular.y);
		SymbolTable.add_variable(lightNum + "_colSpcB", DEMO->m_lightManager.light[i]->colSpecular.z);

		SymbolTable.add_variable(lightNum + "_ambientStrength", DEMO->m_lightManager.light[i]->ambientStrength);
		SymbolTable.add_variable(lightNum + "_specularStrength", DEMO->m_lightManager.light[i]->specularStrength);

		SymbolTable.add_variable(lightNum + "_constant", DEMO->m_lightManager.light[i]->constant);
		SymbolTable.add_variable(lightNum + "_linear", DEMO->m_lightManager.light[i]->linear);
		SymbolTable.add_variable(lightNum + "_quadratic", DEMO->m_lightManager.light[i]->quadratic);
		
	}

	// Graphic constants
	SymbolTable.add_variable("vpWidth", GLDRV->exprTk_current_viewport.width);
	SymbolTable.add_variable("vpHeight", GLDRV->exprTk_current_viewport.height);
	SymbolTable.add_variable("aspectRatio", GLDRV->exprTk_current_viewport.aspect_ratio);

	// Fbo constants
	SymbolTable.add_variable("fbo0Width", GLDRV->fbo[0].width);
	SymbolTable.add_variable("fbo0Height", GLDRV->fbo[0].height);
	SymbolTable.add_variable("fbo1Width", GLDRV->fbo[1].width);
	SymbolTable.add_variable("fbo1Height", GLDRV->fbo[1].height);
	SymbolTable.add_variable("fbo2Width", GLDRV->fbo[2].width);
	SymbolTable.add_variable("fbo2Height", GLDRV->fbo[2].height);
	SymbolTable.add_variable("fbo3Width", GLDRV->fbo[3].width);
	SymbolTable.add_variable("fbo3Height", GLDRV->fbo[3].height);
	SymbolTable.add_variable("fbo4Width", GLDRV->fbo[4].width);
	SymbolTable.add_variable("fbo4Height", GLDRV->fbo[4].height);

	SymbolTable.add_variable("fbo5Width", GLDRV->fbo[5].width);
	SymbolTable.add_variable("fbo5Height", GLDRV->fbo[5].height);
	SymbolTable.add_variable("fbo6Width", GLDRV->fbo[6].width);
	SymbolTable.add_variable("fbo6Height", GLDRV->fbo[6].height);
	SymbolTable.add_variable("fbo7Width", GLDRV->fbo[7].width);
	SymbolTable.add_variable("fbo7Height", GLDRV->fbo[7].height);
	SymbolTable.add_variable("fbo8Width", GLDRV->fbo[8].width);
	SymbolTable.add_variable("fbo8Height", GLDRV->fbo[8].height);
	SymbolTable.add_variable("fbo9Width", GLDRV->fbo[9].width);
	SymbolTable.add_variable("fbo9Height", GLDRV->fbo[9].height);

	SymbolTable.add_variable("fbo10Width", GLDRV->fbo[10].width);
	SymbolTable.add_variable("fbo10Height", GLDRV->fbo[10].height);
	SymbolTable.add_variable("fbo11Width", GLDRV->fbo[11].width);
	SymbolTable.add_variable("fbo11Height", GLDRV->fbo[11].height);
	SymbolTable.add_variable("fbo12Width", GLDRV->fbo[12].width);
	SymbolTable.add_variable("fbo12Height", GLDRV->fbo[12].height);
	SymbolTable.add_variable("fbo13Width", GLDRV->fbo[13].width);
	SymbolTable.add_variable("fbo13Height", GLDRV->fbo[13].height);
	SymbolTable.add_variable("fbo14Width", GLDRV->fbo[14].width);
	SymbolTable.add_variable("fbo14Height", GLDRV->fbo[14].height);

	SymbolTable.add_variable("fbo15Width", GLDRV->fbo[15].width);
	SymbolTable.add_variable("fbo15Height", GLDRV->fbo[15].height);
	SymbolTable.add_variable("fbo16Width", GLDRV->fbo[16].width);
	SymbolTable.add_variable("fbo16Height", GLDRV->fbo[16].height);
	SymbolTable.add_variable("fbo17Width", GLDRV->fbo[17].width);
	SymbolTable.add_variable("fbo17Height", GLDRV->fbo[17].height);
	SymbolTable.add_variable("fbo18Width", GLDRV->fbo[18].width);
	SymbolTable.add_variable("fbo18Height", GLDRV->fbo[18].height);
	SymbolTable.add_variable("fbo19Width", GLDRV->fbo[19].width);
	SymbolTable.add_variable("fbo19Height", GLDRV->fbo[19].height);

	// Fbo 20 to 24 are of a fixed size
	SymbolTable.add_constant("fbo20Width", GLDRV->fbo[20].width);
	SymbolTable.add_constant("fbo20Height", GLDRV->fbo[20].height);
	SymbolTable.add_constant("fbo21Width", GLDRV->fbo[21].width);
	SymbolTable.add_constant("fbo21Height", GLDRV->fbo[21].height);
	SymbolTable.add_constant("fbo22Width", GLDRV->fbo[22].width);
	SymbolTable.add_constant("fbo22Height", GLDRV->fbo[22].height);
	SymbolTable.add_constant("fbo23Width", GLDRV->fbo[23].width);
	SymbolTable.add_constant("fbo23Height", GLDRV->fbo[23].height);
	SymbolTable.add_constant("fbo24Width", GLDRV->fbo[24].width);
	SymbolTable.add_constant("fbo24Height", GLDRV->fbo[24].height);
}

bool MathDriver::compileFormula()
{
	Expression.register_symbol_table(SymbolTable);
	if (!Parser.compile(expression, Expression)) {
		Logger::error("Error in formula, please check expression: %s", expression.c_str());
		return false;
	}
	return true;
}


