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

std::string sTest::debug() {
	return "[ testSection id: " + this->identifier + " layer:" + std::to_string(this->layer) + " ]\n";
}
