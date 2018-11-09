#include "main.h"


sGLSLShaderUnbind::sGLSLShaderUnbind()
{
	type = SectionType::GLSLShaderUnbind;
}

void sGLSLShaderUnbind::load() {
	LOG->Info(LOG_HIGH, "  > UNBIND LOAD has been called!");
}

void sGLSLShaderUnbind::init() {
	LOG->Info(LOG_LOW, "  > UNBIND INIT has been called!");
}

void sGLSLShaderUnbind::exec() {
	LOG->Info(LOG_LOW, "  > UNBIND RENDER has been called!");
}


void sGLSLShaderUnbind::end() {
	LOG->Info(LOG_HIGH, "  > UNBIND END has been called!");

}
