#include "main.h"
#include "core/shadervars.h"

struct sTest : public Section{
public:
	sTest();
	bool		load();
	void		init();
	void		exec();
	void		end();
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

void sTest::end() {
}

std::string sTest::debug() {
	std::stringstream ss;
	ss << "+ Test id: " << identifier << " layer: " << layer << std::endl;
	return ss.str();
}
