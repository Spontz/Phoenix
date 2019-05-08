#include "main.h"


sGLSLShaderUnbind::sGLSLShaderUnbind() {
	type = SectionType::GLSLShaderUnbind;
}

void sGLSLShaderUnbind::load() {
	
}

void sGLSLShaderUnbind::init() {
	
}

void sGLSLShaderUnbind::exec() {
	DEMO->shaderManager.unbindShaders();
}


void sGLSLShaderUnbind::end() {

}
