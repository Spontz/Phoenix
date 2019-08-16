#include "main.h"


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

int error = 0;

void sTest::exec() {
	local = (test_section *)this->vars;

	LOG->Error("Test error %d", error);
	error++;
}

void sTest::end() {
}
