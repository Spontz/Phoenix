// mathdriver.h
// Spontz Demogroup

#ifndef MATHDRIVER_H
#define MATHDRIVER_H


using namespace std;

// ******************************************************************
//typedef exprtk::symbol_table<double> symbol_table_t;
//typedef exprtk::expression<double>     expression_t;
//typedef exprtk::parser<double>             parser_t;
// ******************************************************************

class mathDriver {
	
public:
	
	string expression;
	exprtk::symbol_table<float> SymbolTable;
	exprtk::expression<float>	Expression;
	exprtk::parser<float> Parser;

	mathDriver();

private:

};

#endif