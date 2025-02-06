// MathDriver.h
// Spontz Demogroup

#pragma once

namespace Phoenix {

	class MathDriver final {

	public:
		MathDriver(Section* pSection);

	public:
		bool isValid() const { return valid; }
		bool compileFormula();
		void executeFormula();

	public:
		std::string					expression;
		exprtk::symbol_table<float>	SymbolTable;
		exprtk::expression<float>	Expression;
		exprtk::parser<float>		Parser;

	private:
		bool						valid; // Is the expression valid?
	};
}