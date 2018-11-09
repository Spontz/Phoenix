#include "main.h"

void sGLSLShaderBind::load() {
}

void sGLSLShaderBind::init() {
	LOG->Info("  > BIND INIT has been called!");

}

void sGLSLShaderBind::exec() {
	GLDRV->render(DEMO->runTime);

}

sGLSLShaderBind::sGLSLShaderBind()
{
	type = SectionType::GLSLShaderBind;
}

void sGLSLShaderBind::end() {
	LOG->Info("  > BIND END has been called!");

}
