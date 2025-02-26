# 2018-2025, Spontz

cmake_minimum_required(VERSION 3.18)
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

project(Phoenix)
# set(CMAKE_CXX_STANDARD latest)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
if (_cpp_latest_flag_supported)
	add_compile_options("/std:c++latest")
endif()

# TODO/FIXME: For some reason, C++latest is not compatible with C17, so disabling this
#include(CheckCCompilerFlag)
#CHECK_CXX_COMPILER_FLAG("/std:c17" _c17_flag_supported)
#if (_c17_flag_supported)
#	add_compile_options("/std:c17")
#endif()

message("\n-- Building main project...")

if (MSVC)
	add_compile_options(/MP)
	add_compile_options(-bigobj)
endif()

# HACK: Hide some warnings
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

# Library Dependencies
find_package(OpenGL REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(FFMPEG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(kissfft CONFIG REQUIRED)

# Set Startup project
if(MSVC)
	set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT "Launcher")
endif()

# Declare Visual Studio project virtual folders
function(create_msvc_groups)
	if(MSVC_IDE)
		# Macro to preserve source files hierarchy in the IDE
		macro(GroupSources curdir)
			file(GLOB children RELATIVE ${SubProject_SOURCE_DIR}/${curdir} ${SubProject_SOURCE_DIR}/${curdir}/*)

			foreach(child ${children})
				if(IS_DIRECTORY ${SubProject_SOURCE_DIR}/${curdir}/${child})
					GroupSources(${curdir}/${child})
				else()
					string(REPLACE "/" "\\" groupname ${curdir})
					string(REPLACE "src" "Sources" groupname ${groupname})
					source_group(${groupname} FILES ${SubProject_SOURCE_DIR}/${curdir}/${child})
				endif()
			endforeach()
		endmacro()

		# Run macro
		GroupSources(src)
	endif()
endFunction()


add_subdirectory(Engine)
# add_subdirectory(Editor)
add_subdirectory(Launcher)