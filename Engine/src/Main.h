// main.h
// Spontz Demogroup

#pragma once

#include "libs.h"

//#define PROFILE_PHOENIX

#pragma warning(disable: 4100)

static constexpr uint32_t PHOENIX_MAJOR_VERSION = 2;
static constexpr uint32_t PHOENIX_MINOR_VERSION = 3;
static constexpr uint32_t PHOENIX_BUILD_VERSION = 17;

static constexpr uint32_t FBO_BUFFERS = 25;
static constexpr uint32_t FBO_MAX_COLOR_ATTACHMENTS = 4;
static constexpr uint32_t EFXBLOOM_FBO_BUFFERS = 2;
static constexpr uint32_t EFXACCUM_FBO_BUFFERS = 2;
static constexpr uint32_t MULTIPURPOSE_VARS = 10;
static constexpr uint32_t MAX_BEATS = 5;

// Warning! 'FBO_BUFFERS' should not be less than 25, if you want to set less than 25,
// please check the variable 'scriptCommand' contents in 'demokernel.cpp' file.
static_assert(FBO_BUFFERS >= 25);

#define BIT(x) (1 << x)
#define PX_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#include "core/Demokernel.h"
#include "core/resource/Resource.h"
