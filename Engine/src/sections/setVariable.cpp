#include "main.h"
#include "core/drivers/mathdriver.h"

namespace Phoenix {

	struct sSetVariable : public Section {
	public:
		sSetVariable();
		bool		load();
		void		init();
		void		exec();
		void		destroy();
		void		loadDebugStatic();
		std::string debug();

	private:
		MathDriver* m_pExpr = nullptr;	// The equations with the variabale changes
	};

	// ******************************************************************

	Section* instance_setVariable()
	{
		return new sSetVariable();
	}

	sSetVariable::sSetVariable()
	{
		type = SectionType::SetVariable;
	}


	bool sSetVariable::load()
	{
		// Register the variables
		m_pExpr = new MathDriver(this);
		std::string string_expr;
		for (int i = 0; i < strings.size(); i++)
			string_expr += strings[i];
		m_pExpr->expression = string_expr;															// Loads the expression, properly composed
		m_pExpr->Expression.register_symbol_table(m_pExpr->SymbolTable);
		if (!m_pExpr->compileFormula())
			return false;

		return true;
	}

	void sSetVariable::init()
	{
	}

	void sSetVariable::exec()
	{
		// Evaluate the expression
		m_pExpr->Expression.value();
	}

	void sSetVariable::destroy()
	{
	}

	void sSetVariable::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "  expression: " << m_pExpr->expression << std::endl;
		debugStatic = ss.str();
	}

	std::string sSetVariable::debug()
	{
		return debugStatic;
	}
}