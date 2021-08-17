// MathDriver.h
// Spontz Demogroup

#pragma once

namespace Phoenix {

	class MathDriver final {

	public:
		MathDriver(Section* pSection);

	public:
		bool compileFormula();

	public:
		std::string					expression;
		exprtk::symbol_table<float>	SymbolTable;
		exprtk::expression<float>	Expression;
		exprtk::parser<float>		Parser;
	};
}