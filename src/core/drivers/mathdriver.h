// mathdriver.h
// Spontz Demogroup

#ifndef MATHDRIVER_H
#define MATHDRIVER_H

class mathDriver {
	
public:
	
	string expression;
	exprtk::symbol_table<float> SymbolTable;
	exprtk::expression<float>	Expression;
	exprtk::parser<float> Parser;

	mathDriver(Section *sec);
	bool compileFormula();

private:

};

#endif
