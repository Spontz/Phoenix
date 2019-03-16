#include "main.h"

sLoading::sLoading() {
	type = SectionType::Loading;
}

void sLoading::load() {
	LOG->Info(LOG_HIGH, "  > LOADING LOAD() has been called!");
	this->loaded = TRUE;
}

void sLoading::init() {
	LOG->Info(LOG_LOW, "  > LOADING INIT() has been called!");
}

void sLoading::exec() {
	LOG->Info(LOG_LOW, "  > LOADING EXEC() has been called!");
}

void sLoading::end() {
	LOG->Info(LOG_HIGH, "  > LOADING END() has been called!");
}
