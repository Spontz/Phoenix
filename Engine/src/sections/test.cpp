#include "main.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {
	struct sTest : public Section {
	public:
		sTest();
		bool		load();
		void		init();
		void		exec();
		void		destroy();
		std::string debug();

	private:

	};


	// ******************************************************************

	Section* instance_test() {
		return new sTest();
	}

	sTest::sTest() {
		type = SectionType::Test;
	}

	bool sTest::load() {
		return true;
	}

	void sTest::init() {
	}

	void sTest::exec() {

	}

	void sTest::destroy() {
	}

	std::string sTest::debug() {
		std::stringstream ss;
		return ss.str();
	}
}