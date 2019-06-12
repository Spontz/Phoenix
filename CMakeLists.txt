# Copyright (C) 2018 Spontz

cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(Phoenix)

# Configure a few project settings
#Hide console and allow main() to be the entry point
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#Unicode files
SET(BUILD_UNICODE ON CACHE BOOL "Unicode")
add_definitions(-DUNICODE -D_UNICODE)
#Avoid some warnings... :P
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
#Multiprocessor compiling
set(CMAKE_C_FLAGS "/MP ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "/MP ${CMAKE_CXX_FLAGS}")
#ISO standard
set(CMAKE_C_FLAGS "/std:c++14 ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "/std:c++14 ${CMAKE_CXX_FLAGS}")
set(CMAKE_C_FLAGS "/bigobj ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "/bigobj ${CMAKE_CXX_FLAGS}")

# Add .lib files
if(CMAKE_CL_64)
	link_directories(${CMAKE_SOURCE_DIR}/lib/lib64)
else()
	link_directories(${CMAKE_SOURCE_DIR}/lib/lib32)
endif()


# Add source files
file(GLOB_RECURSE SOURCE_FILES 
	${CMAKE_SOURCE_DIR}/src/*.c
	${CMAKE_SOURCE_DIR}/src/*.cpp)
	
# Add header files
file(GLOB_RECURSE HEADER_FILES 
	${CMAKE_SOURCE_DIR}/src/*.h
	${CMAKE_SOURCE_DIR}/src/*.hpp)

# Add resource files (icons)
file(GLOB_RECURSE RESOURCE_FILES 
	${CMAKE_SOURCE_DIR}/res/*.rc)

# Define the executable
add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})

# We need a CMAKE_DIR with some code to find external dependencies
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

#######################################
# LOOK for the packages that we need! #
#######################################

# OpenGL
find_package(OpenGL REQUIRED)

# GLM
find_package(GLM REQUIRED)
message(STATUS "GLM included at ${GLM_INCLUDE_DIR}")

# GLFW
find_package(GLFW3 REQUIRED)
message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")

# ASSIMP
find_package(ASSIMP REQUIRED)
message(STATUS "Found ASSIMP in ${ASSIMP_INCLUDE_DIR}")

# STB_IMAGE
add_library(STB_IMAGE "thirdparty/stb_image.cpp")

# GLAD
add_library(GLAD "thirdparty/glad.c")

# BASS and FREETYPE
if(CMAKE_CL_64)
	link_directories(${CMAKE_SOURCE_DIR}/lib/lib64)
else()
	link_directories(${CMAKE_SOURCE_DIR}/lib/lib32)
endif()

# Put all libraries into a variable
set(LIBS glfw3 opengl32 ws2_32 assimp bass freetype STB_IMAGE GLAD)

# Define the include DIRs
include_directories(
	"${CMAKE_SOURCE_DIR}/src"
	"${CMAKE_SOURCE_DIR}/include"
)

# Define the link libraries
target_link_libraries(${PROJECT_NAME} ${LIBS})

#Set the working directory of the project
set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Create virtual folders to make it look nicer in VS
if(MSVC_IDE)
	# Macro to preserve source files hierarchy in the IDE
	macro(GroupSources curdir)
		file(GLOB children RELATIVE ${PROJECT_SOURCE_DIR}/${curdir} ${PROJECT_SOURCE_DIR}/${curdir}/*)

		foreach(child ${children})
			if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/${curdir}/${child})
				GroupSources(${curdir}/${child})
			else()
				string(REPLACE "/" "\\" groupname ${curdir})
				string(REPLACE "src" "Sources" groupname ${groupname})
				source_group(${groupname} FILES ${PROJECT_SOURCE_DIR}/${curdir}/${child})
			endif()
		endforeach()
	endmacro()

	# Run macro
	GroupSources(src)
endif()

# Copy dlls
if(CMAKE_CL_64)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${PROJECT_SOURCE_DIR}/dlls/dll64"
		$<TARGET_FILE_DIR:${PROJECT_NAME}>)
else()
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${PROJECT_SOURCE_DIR}/dlls/dll32"
		$<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif()

