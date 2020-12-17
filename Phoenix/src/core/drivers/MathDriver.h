// MathDriver.h
// Spontz Demogroup

#pragma once

class MathDriver {
	
public:
	
	std::string					expression;
	exprtk::symbol_table<float>	SymbolTable;
	exprtk::expression<float>	Expression;
	exprtk::parser<float>		Parser;

	MathDriver(Section *sec);
	bool compileFormula();

private:

};
