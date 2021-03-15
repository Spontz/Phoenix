# 2018-2021, Spontz

cmake_minimum_required(VERSION 3.8)
# Options to try...
# set(VCPKG_CRT_LINKAGE static)
# set(VCPKG_LIBRARY_LINKAGE static)
# set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "Vcpkg target triplet (ex. x86-windows)")

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

project(Phoenix)
set(CMAKE_CXX_STANDARD 20)
message("\n-- Building main project...")

if (MSVC)
	add_compile_options(/MP)
	add_compile_options(-bigobj)
	add_compile_options(
		"$<$<CONFIG:Debug>:/MTd>"
		"$<$<CONFIG:RelWithDebInfo>:/MT>"
		"$<$<CONFIG:Release>:/MT>"
		"$<$<CONFIG:MinSizeRel>:/MT>"
	)
endif()

# HACK: Hide some warnings
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

# Bass: Create library
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
#find_package(imgui CONFIG REQUIRED)

# Set Startup project
if(MSVC)
	set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT "Launcher")
endif()


add_subdirectory(Engine)
# add_subdirectory(Editor)
add_subdirectory(Launcher)