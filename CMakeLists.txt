# 2018-2021, Spontz

cmake_minimum_required(VERSION 3.8)
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

project(Phoenix)
set(CMAKE_CXX_STANDARD 17)
message("\n-- Building main project...")

if (MSVC)
	add_compile_options(/MP)
	add_compile_options(-bigobj)
endif()

# HACK: Hide some warnings
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

# BASS: Create library
add_library(bass STATIC IMPORTED)
set_target_properties(bass PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/third_party/bass24/bass24/c/x64/bass.lib")

# BASS: include libs
link_directories(${CMAKE_SOURCE_DIR}/third_party/bass24/bass24/c/x64)

# Library Dependencies
find_package(OpenGL REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(Freetype REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(FFMPEG REQUIRED)
find_package(imgui CONFIG REQUIRED)

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