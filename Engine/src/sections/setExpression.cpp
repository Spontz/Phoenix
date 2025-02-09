#include "main.h"
#include "core/drivers/mathdriver.h"

namespace Phoenix {

	class sSetExpression final : public Section {
	public:
		sSetExpression();
		~sSetExpression();

	public:
		bool		load();
		void		init();
		void		exec();
		void		loadDebugStatic();
		std::string debug();

	private:
		MathDriver* m_pExpr = nullptr;	// The equations with the variabale changes
	};

	// ******************************************************************

	Section* instance_setExpression()
	{
		return new sSetExpression();
	}

	sSetExpression::sSetExpression()
	{
		type = SectionType::SetExpression;
	}

	sSetExpression::~sSetExpression()
	{
		if (m_pExpr)
			delete m_pExpr;
	}

	bool sSetExpression::load()
	{
		if (expressionRun == "") {
			Logger::error("Set Expression [{}]: Expression does not exist or is empty, 1 expression is needed", identifier);
			return false;
		}
		// Register the variables
		m_pExpr = new MathDriver(this);
		m_pExpr->expression = expressionRun;
		m_pExpr->Expression.register_symbol_table(m_pExpr->SymbolTable);
		if (!m_pExpr->compileFormula())
			Logger::error("Set Expression [{}]: Error while compiling the expression, default values used", identifier);

		return true;
	}

	void sSetExpression::init()
	{
	}

	void sSetExpression::exec()
	{
		// Evaluate the expression
		m_pExpr->executeFormula();
	}

	void sSetExpression::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "Expression is: " << (m_pExpr->isValid() ? "Valid" : "Faulty or Empty") << std::endl;
		debugStatic = ss.str();
	}

	std::string sSetExpression::debug()
	{
		std::stringstream ss;
		ss << debugStatic;
		ss << "Variable values:" << std::endl;
		for (int32_t i = 0; i < MULTIPURPOSE_VARS; ++i) {
			ss << "var" << i << ": " << DEMO->m_fVar[i] << std::endl;
		}

		/*
		// This is very hardcore: many variables are shown here
		std::vector<std::string> variable_list;
		m_pExpr->SymbolTable.get_variable_list(variable_list);
		for (const auto& var_name : variable_list)
		{
			float value = m_pExpr->SymbolTable.variable_ref(var_name); 
			ss << var_name << ": " << value << std::endl;
		}
		*/
		return ss.str();
	}
}