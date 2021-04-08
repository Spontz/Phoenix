// MathDriver.cpp
// Spontz Demogroup
// fastmath library: https://archive.codeplex.com/?p=fastmathparser

#include "main.h"
#include "core/drivers/MathDriver.h"

namespace Phoenix {

	MathDriver::MathDriver(Section* sec) {
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
		if (DEMO->m_pActiveCamera != NULL) {
			SymbolTable.add_variable("cam_posX", DEMO->m_pActiveCamera->m_Position.x);
			SymbolTable.add_variable("cam_posY", DEMO->m_pActiveCamera->m_Position.y);
			SymbolTable.add_variable("cam_posZ", DEMO->m_pActiveCamera->m_Position.z);

			SymbolTable.add_variable("cam_upX", DEMO->m_pActiveCamera->m_Up.x);
			SymbolTable.add_variable("cam_upY", DEMO->m_pActiveCamera->m_Up.y);
			SymbolTable.add_variable("cam_upZ", DEMO->m_pActiveCamera->m_Up.z);

			SymbolTable.add_variable("cam_yaw", DEMO->m_pActiveCamera->m_Yaw);
			SymbolTable.add_variable("cam_pitch", DEMO->m_pActiveCamera->m_Pitch);
			SymbolTable.add_variable("cam_roll", DEMO->m_pActiveCamera->m_Roll); 
			SymbolTable.add_variable("cam_fov", DEMO->m_pActiveCamera->m_Fov);
		}
		else {
			SymbolTable.add_constant("cam_posX", 0);
			SymbolTable.add_constant("cam_posY", 0);
			SymbolTable.add_constant("cam_posZ", 0);

			SymbolTable.add_constant("cam_upX", 0);
			SymbolTable.add_constant("cam_upY", 0);
			SymbolTable.add_constant("cam_upZ", 1);

			SymbolTable.add_constant("cam_yaw", 0);
			SymbolTable.add_constant("cam_pitch", 0);
			SymbolTable.add_constant("cam_roll", 0);
			SymbolTable.add_constant("cam_fov", 0);			
		}

		// Light values
		for (int i = 0; i < DEMO->m_lightManager.light.size(); i++) {
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
		SymbolTable.add_variable("vpWidth", GLDRV->m_exprtkCurrentViewport.width);
		SymbolTable.add_variable("vpHeight", GLDRV->m_exprtkCurrentViewport.height);
		SymbolTable.add_variable("aspectRatio", GLDRV->m_exprtkCurrentViewport.aspect_ratio);

		// Fbo constants
		SymbolTable.add_variable("fbo0Width", GLDRV->fboConfig[0].width);
		SymbolTable.add_variable("fbo0Height", GLDRV->fboConfig[0].height);
		SymbolTable.add_variable("fbo1Width", GLDRV->fboConfig[1].width);
		SymbolTable.add_variable("fbo1Height", GLDRV->fboConfig[1].height);
		SymbolTable.add_variable("fbo2Width", GLDRV->fboConfig[2].width);
		SymbolTable.add_variable("fbo2Height", GLDRV->fboConfig[2].height);
		SymbolTable.add_variable("fbo3Width", GLDRV->fboConfig[3].width);
		SymbolTable.add_variable("fbo3Height", GLDRV->fboConfig[3].height);
		SymbolTable.add_variable("fbo4Width", GLDRV->fboConfig[4].width);
		SymbolTable.add_variable("fbo4Height", GLDRV->fboConfig[4].height);

		SymbolTable.add_variable("fbo5Width", GLDRV->fboConfig[5].width);
		SymbolTable.add_variable("fbo5Height", GLDRV->fboConfig[5].height);
		SymbolTable.add_variable("fbo6Width", GLDRV->fboConfig[6].width);
		SymbolTable.add_variable("fbo6Height", GLDRV->fboConfig[6].height);
		SymbolTable.add_variable("fbo7Width", GLDRV->fboConfig[7].width);
		SymbolTable.add_variable("fbo7Height", GLDRV->fboConfig[7].height);
		SymbolTable.add_variable("fbo8Width", GLDRV->fboConfig[8].width);
		SymbolTable.add_variable("fbo8Height", GLDRV->fboConfig[8].height);
		SymbolTable.add_variable("fbo9Width", GLDRV->fboConfig[9].width);
		SymbolTable.add_variable("fbo9Height", GLDRV->fboConfig[9].height);

		SymbolTable.add_variable("fbo10Width", GLDRV->fboConfig[10].width);
		SymbolTable.add_variable("fbo10Height", GLDRV->fboConfig[10].height);
		SymbolTable.add_variable("fbo11Width", GLDRV->fboConfig[11].width);
		SymbolTable.add_variable("fbo11Height", GLDRV->fboConfig[11].height);
		SymbolTable.add_variable("fbo12Width", GLDRV->fboConfig[12].width);
		SymbolTable.add_variable("fbo12Height", GLDRV->fboConfig[12].height);
		SymbolTable.add_variable("fbo13Width", GLDRV->fboConfig[13].width);
		SymbolTable.add_variable("fbo13Height", GLDRV->fboConfig[13].height);
		SymbolTable.add_variable("fbo14Width", GLDRV->fboConfig[14].width);
		SymbolTable.add_variable("fbo14Height", GLDRV->fboConfig[14].height);

		SymbolTable.add_variable("fbo15Width", GLDRV->fboConfig[15].width);
		SymbolTable.add_variable("fbo15Height", GLDRV->fboConfig[15].height);
		SymbolTable.add_variable("fbo16Width", GLDRV->fboConfig[16].width);
		SymbolTable.add_variable("fbo16Height", GLDRV->fboConfig[16].height);
		SymbolTable.add_variable("fbo17Width", GLDRV->fboConfig[17].width);
		SymbolTable.add_variable("fbo17Height", GLDRV->fboConfig[17].height);
		SymbolTable.add_variable("fbo18Width", GLDRV->fboConfig[18].width);
		SymbolTable.add_variable("fbo18Height", GLDRV->fboConfig[18].height);
		SymbolTable.add_variable("fbo19Width", GLDRV->fboConfig[19].width);
		SymbolTable.add_variable("fbo19Height", GLDRV->fboConfig[19].height);

		// Fbo 20 to 24 are of a fixed size
		SymbolTable.add_constant("fbo20Width", GLDRV->fboConfig[20].width);
		SymbolTable.add_constant("fbo20Height", GLDRV->fboConfig[20].height);
		SymbolTable.add_constant("fbo21Width", GLDRV->fboConfig[21].width);
		SymbolTable.add_constant("fbo21Height", GLDRV->fboConfig[21].height);
		SymbolTable.add_constant("fbo22Width", GLDRV->fboConfig[22].width);
		SymbolTable.add_constant("fbo22Height", GLDRV->fboConfig[22].height);
		SymbolTable.add_constant("fbo23Width", GLDRV->fboConfig[23].width);
		SymbolTable.add_constant("fbo23Height", GLDRV->fboConfig[23].height);
		SymbolTable.add_constant("fbo24Width", GLDRV->fboConfig[24].width);
		SymbolTable.add_constant("fbo24Height", GLDRV->fboConfig[24].height);
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
}