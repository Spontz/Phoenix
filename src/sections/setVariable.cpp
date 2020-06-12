#include "main.h"

typedef struct {
	mathDriver	*expr;	// The equations with the variabale changes

} setVariable_section;

static setVariable_section *local;

sSetVariable::sSetVariable() {
	type = SectionType::SetVariable;
}



bool sSetVariable::load() {
	// script validation
	local = (setVariable_section*)malloc(sizeof(setVariable_section));
	this->vars = (void *)local;

	// Register the variables
	local->expr = new mathDriver(this);
	std::string expr;
	for (int i = 0; i < this->strings.size(); i++)
		expr += this->strings[i];
	local->expr->expression = expr;															// Loads the expression, properly composed
	local->expr->Expression.register_symbol_table(local->expr->SymbolTable);
	if (!local->expr->compileFormula())
		return false;

	return true;
}

void sSetVariable::init() {
}

void sSetVariable::exec() {
	local = (setVariable_section*)this->vars;
	
	// Evaluate the expression
	local->expr->Expression.value();
}

void sSetVariable::end() {
}

std::string sSetVariable::debug() {
	local = (setVariable_section*)this->vars;

	std::string msg;
	msg = "[ setVariable id: " + this->identifier + " layer:" + std::to_string(this->layer) + " ]\n";
	msg += " " + local->expr->expression + "\n";
	return msg;
}
