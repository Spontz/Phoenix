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
	return "[ testSection id: " + identifier + " layer:" + std::to_string(layer) + " ]\n";
}
