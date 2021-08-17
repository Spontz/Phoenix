#pragma once

#ifdef PHOENIX_MAIN
#define STB_IMAGE_IMPLEMENTATION
#endif

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL

#include "core/drivers/net/dyad.h"
#include <windows.h>
#include <glad/glad.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <imgui.h>

#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/polar_coordinates.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/ext.hpp>

#include <exprtk.hpp>

#include <stb_image.h>

#include <assimp/version.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <bass.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}
