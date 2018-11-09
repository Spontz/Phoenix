#include "main.h"

void sGLSLShaderUnbind::load() {
	LOG->Info("  > glgshShaderUnbind loader has been called!");
}

void sGLSLShaderUnbind::init() {
	LOG->Info("  > UNBIND INIT has been called!");
}

void sGLSLShaderUnbind::exec() {
	LOG->Info("  > UNBIND RENDER has been called!");
}

sGLSLShaderUnbind::sGLSLShaderUnbind()
{
	type = SectionType::GLSLShaderUnbind;
}

void sGLSLShaderUnbind::end() {
	LOG->Info("  > UNBIND END has been called!");

}
