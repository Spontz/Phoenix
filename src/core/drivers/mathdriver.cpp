// mathdriver.cpp
// Spontz Demogroup
// fastmath library: https://archive.codeplex.com/?p=fastmathparser


//#include <fastmathparser/exprtk.hpp>
#include "main.h"


mathDriver::mathDriver(Section *sec) {
	expression = "";
	
	// Add the variables that will be used by all sections
	SymbolTable.add_variable("t", sec->runTime);
	SymbolTable.add_variable("tend", sec->duration);
	SymbolTable.add_variable("tdemo", DEMO->runTime);
	SymbolTable.add_variable("beat", DEMO->beat);
	SymbolTable.add_variable("fps", DEMO->fps);

	// Camera values
	if (DEMO->camera != NULL) {
		SymbolTable.add_variable("cam_pos_x", DEMO->camera->Position.x);
		SymbolTable.add_variable("cam_pos_y", DEMO->camera->Position.y);
		SymbolTable.add_variable("cam_pos_z", DEMO->camera->Position.z);

		SymbolTable.add_variable("cam_up_x", DEMO->camera->Up.x);
		SymbolTable.add_variable("cam_up_y", DEMO->camera->Up.y);
		SymbolTable.add_variable("cam_up_z", DEMO->camera->Up.z);

		SymbolTable.add_variable("cam_yaw", DEMO->camera->Yaw);
		SymbolTable.add_variable("cam_pitch", DEMO->camera->Pitch);
		SymbolTable.add_variable("cam_zoom", DEMO->camera->Zoom);
	}
	else {
		SymbolTable.add_constant("cam_pos_x", 0.0f);
		SymbolTable.add_constant("cam_pos_y", 0.0f);
		SymbolTable.add_constant("cam_pos_z", 0.0f);

		SymbolTable.add_constant("cam_up_x", 0.0f);
		SymbolTable.add_constant("cam_up_y", 0.0f);
		SymbolTable.add_constant("cam_up_z", 100.0f);

		SymbolTable.add_constant("cam_yaw", 0.0f);
		SymbolTable.add_constant("cam_pitch", 0.0f);
		SymbolTable.add_constant("cam_zoom", 0.0f);
	}

	// Graphic constants
	SymbolTable.add_constant("vpWidth", (float)GLDRV->vpWidth);
	SymbolTable.add_constant("vpHeight", (float)GLDRV->vpHeight);
	SymbolTable.add_constant("aspectRatio", (float)GLDRV->AspectRatio);

	// Fbo constants
	SymbolTable.add_constant("fbo0Width", (float)GLDRV->fbo[0].width);
	SymbolTable.add_constant("fbo0Height", (float)GLDRV->fbo[0].height);
	SymbolTable.add_constant("fbo1Width", (float)GLDRV->fbo[1].width);
	SymbolTable.add_constant("fbo1Height", (float)GLDRV->fbo[1].height);
	SymbolTable.add_constant("fbo2Width", (float)GLDRV->fbo[2].width);
	SymbolTable.add_constant("fbo2Height", (float)GLDRV->fbo[2].height);
	SymbolTable.add_constant("fbo3Width", (float)GLDRV->fbo[3].width);
	SymbolTable.add_constant("fbo3Height", (float)GLDRV->fbo[3].height);
	SymbolTable.add_constant("fbo4Width", (float)GLDRV->fbo[4].width);
	SymbolTable.add_constant("fbo4Height", (float)GLDRV->fbo[4].height);

	SymbolTable.add_constant("fbo5Width", (float)GLDRV->fbo[5].width);
	SymbolTable.add_constant("fbo5Height", (float)GLDRV->fbo[5].height);
	SymbolTable.add_constant("fbo6Width", (float)GLDRV->fbo[6].width);
	SymbolTable.add_constant("fbo6Height", (float)GLDRV->fbo[6].height);
	SymbolTable.add_constant("fbo7Width", (float)GLDRV->fbo[7].width);
	SymbolTable.add_constant("fbo7Height", (float)GLDRV->fbo[7].height);
	SymbolTable.add_constant("fbo8Width", (float)GLDRV->fbo[8].width);
	SymbolTable.add_constant("fbo8Height", (float)GLDRV->fbo[8].height);
	SymbolTable.add_constant("fbo9Width", (float)GLDRV->fbo[9].width);
	SymbolTable.add_constant("fbo9Height", (float)GLDRV->fbo[9].height);

	SymbolTable.add_constant("fbo10Width", (float)GLDRV->fbo[10].width);
	SymbolTable.add_constant("fbo10Height", (float)GLDRV->fbo[10].height);
	SymbolTable.add_constant("fbo11Width", (float)GLDRV->fbo[11].width);
	SymbolTable.add_constant("fbo11Height", (float)GLDRV->fbo[11].height);
	SymbolTable.add_constant("fbo12Width", (float)GLDRV->fbo[12].width);
	SymbolTable.add_constant("fbo12Height", (float)GLDRV->fbo[12].height);
	SymbolTable.add_constant("fbo13Width", (float)GLDRV->fbo[13].width);
	SymbolTable.add_constant("fbo13Height", (float)GLDRV->fbo[13].height);
	SymbolTable.add_constant("fbo14Width", (float)GLDRV->fbo[14].width);
	SymbolTable.add_constant("fbo14Height", (float)GLDRV->fbo[14].height);

	SymbolTable.add_constant("fbo15Width", (float)GLDRV->fbo[15].width);
	SymbolTable.add_constant("fbo15Height", (float)GLDRV->fbo[15].height);
	SymbolTable.add_constant("fbo16Width", (float)GLDRV->fbo[16].width);
	SymbolTable.add_constant("fbo16Height", (float)GLDRV->fbo[16].height);
	SymbolTable.add_constant("fbo17Width", (float)GLDRV->fbo[17].width);
	SymbolTable.add_constant("fbo17Height", (float)GLDRV->fbo[17].height);
	SymbolTable.add_constant("fbo18Width", (float)GLDRV->fbo[18].width);
	SymbolTable.add_constant("fbo18Height", (float)GLDRV->fbo[18].height);
	SymbolTable.add_constant("fbo19Width", (float)GLDRV->fbo[19].width);
	SymbolTable.add_constant("fbo19Height", (float)GLDRV->fbo[19].height);


	SymbolTable.add_constant("fbo20Width", (float)GLDRV->fbo[20].width);
	SymbolTable.add_constant("fbo20Height", (float)GLDRV->fbo[20].height);
	SymbolTable.add_constant("fbo21Width", (float)GLDRV->fbo[21].width);
	SymbolTable.add_constant("fbo21Height", (float)GLDRV->fbo[21].height);
	SymbolTable.add_constant("fbo22Width", (float)GLDRV->fbo[22].width);
	SymbolTable.add_constant("fbo22Height", (float)GLDRV->fbo[22].height);
	SymbolTable.add_constant("fbo23Width", (float)GLDRV->fbo[23].width);
	SymbolTable.add_constant("fbo23Height", (float)GLDRV->fbo[23].height);
	SymbolTable.add_constant("fbo24Width", (float)GLDRV->fbo[24].width);
	SymbolTable.add_constant("fbo24Height", (float)GLDRV->fbo[24].height);
}

void mathDriver::compileFormula()
{

	Expression.register_symbol_table(SymbolTable);
	if (!Parser.compile(expression, Expression))
		LOG->Error("Error in formula, please check expression: %s", expression.c_str());
}


