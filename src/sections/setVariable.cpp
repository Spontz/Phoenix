#include "main.h"

struct sSetVariable : public Section {
public:
	sSetVariable();
	bool		load();
	void		init();
	void		exec();
	void		end();
	std::string debug();

private:
	mathDriver	*expr;	// The equations with the variabale changes
};

// ******************************************************************

Section* instance_setVariable() {
	return new sSetVariable();
}

sSetVariable::sSetVariable() {
	type = SectionType::SetVariable;
}


bool sSetVariable::load() {
	// Register the variables
	expr = new mathDriver(this);
	std::string string_expr;
	for (int i = 0; i < strings.size(); i++)
		string_expr += strings[i];
	expr->expression = string_expr;															// Loads the expression, properly composed
	expr->Expression.register_symbol_table(expr->SymbolTable);
	if (!expr->compileFormula())
		return false;

	return true;
}

void sSetVariable::init() {
}

void sSetVariable::exec() {
	// Evaluate the expression
	expr->Expression.value();
}

void sSetVariable::end() {
}

std::string sSetVariable::debug() {
	std::string msg;
	msg = "[ setVariable id: " + identifier + " layer:" + std::to_string(layer) + " ]\n";
	msg += " " + expr->expression + "\n";
	return msg;
}
