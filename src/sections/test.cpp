#include "main.h"
#include "core/shadervars.h"

typedef struct {
} test_section;

static test_section *local;


sTest::sTest() {
	type = SectionType::Test;
}

bool sTest::load() {
	local = (test_section*)malloc(sizeof(test_section));
	this->vars = (void *)local;
	return true;
}

void sTest::init() {
}

void sTest::exec() {
	local = (test_section *)this->vars;

}

void sTest::end() {
}

string sTest::debug() {
	return string();
}
