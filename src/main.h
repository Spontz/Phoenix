// main.h
// Spontz Demogroup

#ifndef MAIN_H
#define MAIN_H

#define PHOENIX_MAJOR_VERSION 1
#define PHOENIX_MINOR_VERSION 0
#define PHOENIX_BUILD_VERSION 0

#ifdef PHOENIX_MAIN
	#define STB_IMAGE_IMPLEMENTATION
#endif

#define  GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <exprtk.hpp>
#include <glad/glad.h>
#include <assimp/version.h>

#include "core/demokernel.h"

#include "core/drivers/gldriver.h"
#include "core/drivers/bassdriver.h"
#include "core/drivers/mathdriver.h"
#include "core/drivers/netdriver.h"

#include "core/resource/resource.h"

#include "core/utils/logger.h"
#include "core/utils/utilities.h"

#include "sections/sections.h"

#include "debug/instrumentor.h"

#endif
