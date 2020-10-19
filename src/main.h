// main.h
// Spontz Demogroup

#pragma once

#include "libs.h"

#define PHOENIX_MAJOR_VERSION 1
#define PHOENIX_MINOR_VERSION 2
#define PHOENIX_BUILD_VERSION 1

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
