// main.h
// Spontz Demogroup

#pragma once

#include "libs.h"

// #define PROFILE_PHOENIX

static constexpr uint32_t PHOENIX_MAJOR_VERSION = 1;
static constexpr uint32_t PHOENIX_MINOR_VERSION = 16;
static constexpr uint32_t PHOENIX_BUILD_VERSION = 0;

static constexpr uint32_t FBO_BUFFERS = 25;
static constexpr uint32_t EFXBLOOM_FBO_BUFFERS = 2;
static constexpr uint32_t EFXACCUM_FBO_BUFFERS = 2;
static constexpr uint32_t MULTIPURPOSE_VARS = 10;
static constexpr uint32_t MAX_BEATS = 5;

// Warning! 'FBO_BUFFERS' should not be less than 25, if you want to set less than 25,
// please check the variable 'scriptCommand' contents in 'demokernel.cpp' file.
static_assert(FBO_BUFFERS >= 25);

#include "core/Demokernel.h"
#include "core/resource/Resource.h"
