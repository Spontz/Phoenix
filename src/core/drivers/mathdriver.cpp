// mathdriver.cpp
// Spontz Demogroup
// fastmath library: https://archive.codeplex.com/?p=fastmathparser


//#include <fastmathparser/exprtk.hpp>
#include "main.h"


mathDriver::mathDriver(Section *sec) {
	expression = "";

	// Add the variables that will be used by all sections
	SymbolTable.add_variable("t", sec->runTime);//DEMO->runTime);
	SymbolTable.add_variable("tend", sec->duration);
	SymbolTable.add_variable("beat", DEMO->beat);
	SymbolTable.add_variable("fps", DEMO->fps);
}


