// main.h
// Spontz Demogroup

#ifndef MAIN_H
#define MAIN_H

#define PHOENIX_MAJOR_VERSION 1
#define PHOENIX_MINOR_VERSION 0
#define PHOENIX_BUILD_VERSION 7

#ifdef PHOENIX_MAIN
	#define STB_IMAGE_IMPLEMENTATION
#endif

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL

#include <windows.h>
#include <glad/glad.h>
#include "core/drivers/imGui/imgui_impl_glfw.h"
#include "core/drivers/imGui/imgui_impl_opengl3.h"
#include <imgui.h>

#include <GLFW/glfw3.h>
//#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>

#include <exprtk.hpp>


#include <stb_image.h>

#include <assimp/version.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <bass.h>

// ******************************************************************

#define FBO_BUFFERS				25
#define EFXBLOOM_FBO_BUFFERS	2
#define EFXACCUM_FBO_BUFFERS	2
#define MULTIPURPOSE_VARS		10

// Warning! 'FBO_BUFFERS' should not be less than 25, if you want to set less than 25,
// please check the variable 'scriptCommand' contents in 'demokernel.cpp' file.


// ******************************************************************
// Declarations

class demokernel;
class mathDriver;

// ******************************************************************

#include "core/demokernel.h"
#include "core/utils/logger.h"
#include "core/utils/utilities.h"
#include "core/resource/resource.h"

// TODO:  Borrar las librerias internas
/*


#include "core/drivers/gldriver.h"
#include "core/drivers/bassdriver.h"
#include "core/drivers/mathdriver.h"
#include "core/drivers/netdriver.h"

#include "core/resource/resource.h"


#include "sections/sections.h"

#include "debug/instrumentor.h"
*/

#endif
