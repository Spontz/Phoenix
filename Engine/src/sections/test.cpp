#include "main.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {
	class sTest final : public Section {
	public:
		sTest();
		~sTest();

	public:
		bool		load();
		void		init();
		void		exec();
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

	sTest::~sTest() {
	}

	bool sTest::load() {

		return !DEMO_checkGLError();
	}

	void sTest::init() {
	}

	void sTest::exec() {

	}

	std::string sTest::debug() {
		std::stringstream ss;
		return ss.str();
	}
}