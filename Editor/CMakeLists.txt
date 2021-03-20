# 2018-2021, Spontz

set(SubProject "Editor")
set(SubProject_OutputName "Editor")
set(SubProject_SOURCE_DIR ${PROJECT_SOURCE_DIR}/${SubProject})


# Show some debug info
message("\n-- Building ${SubProject}")
message("  VCKPGG folder is: ${CMAKE_TOOLCHAIN_FILE}")
message("  Project_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")
message("  SubProject_SOURCE_DIR: ${SubProject_SOURCE_DIR}")
message("  Output Name: ${SubProject_OutputName}")

# Hide console and allow main() to be the entry point
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")

# include source files
file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/src/*.ipp ${CMAKE_CURRENT_SOURCE_DIR}/src/*.inl)
file(GLOB_RECURSE RESOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/res/*.rc)

# Create executable
add_executable(${SubProject} ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})
# Extra include folders
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")
include_directories("${CMAKE_SOURCE_DIR}/third_party/bass24/bass24/c")
include_directories(${CMAKE_SOURCE_DIR}) # In order to be able to include files from Engine

# Set project debug working dir
set(DEBUGGER_WD "${CMAKE_SOURCE_DIR}/${SubProject}_WD")
set_target_properties(${SubProject} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${DEBUGGER_WD})
message("  Debugger Work directory is: ${DEBUGGER_WD}")

# Set output name
set_target_properties(${SubProject} PROPERTIES OUTPUT_NAME ${SubProject_OutputName})

# Link libraries
target_link_libraries(${SubProject} PRIVATE Engine)
target_link_libraries(${SubProject} PRIVATE ${OPENGL_LIBRARIES})
target_link_libraries(${SubProject} PRIVATE glm::glm)
target_link_libraries(${SubProject} PRIVATE ${FFMPEG_LIBRARIES})
target_link_libraries(${SubProject} PRIVATE glad::glad)
target_link_libraries(${SubProject} PRIVATE Freetype::Freetype)
target_link_libraries(${SubProject} PRIVATE glfw)
target_link_libraries(${SubProject} PRIVATE ws2_32)
target_link_libraries(${SubProject} PRIVATE assimp::assimp)
target_link_libraries(${SubProject} PRIVATE bass)

# HACK: Workaround for broken Dear ImGui Vcpkg port
if(MSVC)
	target_link_libraries(
		${SubProject}
		PRIVATE
		../../vcpkg/installed/x64-windows-static/$<$<CONFIG:Debug>:debug/lib/imguid>$<$<CONFIG:Release>:lib/imgui>
	)
else()
	target_link_libraries(${SubProject} PRIVATE imgui)
endif()


install(TARGETS ${SubProject} RUNTIME DESTINATION bin)

# Declare Visual Studio project virtual folders
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

# BASS: Copy library after build
add_custom_command(
	TARGET ${SubProject} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	"${PROJECT_SOURCE_DIR}/third_party/bass24/bass24/x64/bass.dll"
	$<TARGET_FILE_DIR:${SubProject}>
)
