#include "main.h"


sGLSLShaderUnbind::sGLSLShaderUnbind() {
	type = SectionType::GLSLShaderUnbind;
}

bool sGLSLShaderUnbind::load() {
	return true;
}

void sGLSLShaderUnbind::init() {
	
}

void sGLSLShaderUnbind::exec() {
	DEMO->shaderManager.unbindShaders();
}


void sGLSLShaderUnbind::end() {

}
