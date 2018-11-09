#include "main.h"

sGLSLShaderBind::sGLSLShaderBind()
{
	type = SectionType::GLSLShaderBind;
}

void sGLSLShaderBind::load() {
	LOG->Info(LOG_HIGH, "  > BIND LOAD has been called!");
}

void sGLSLShaderBind::init() {
	LOG->Info(LOG_LOW, "  > BIND INIT has been called!");
}

void sGLSLShaderBind::exec() {
	GLDRV->render(DEMO->runTime);
}



void sGLSLShaderBind::end() {
	LOG->Info(LOG_HIGH, "  > BIND END has been called!");

}
