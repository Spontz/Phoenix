# 2018-2024, Spontz

set(SubProject "Engine")
set(SubProject_SOURCE_DIR ${PROJECT_SOURCE_DIR}/${SubProject})

# Show some debug info
message("\n-- Building ${SubProject}")
message("  VCKPGG folder is: ${CMAKE_TOOLCHAIN_FILE}")
message("  Project_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")
message("  SubProject_SOURCE_DIR: ${SubProject_SOURCE_DIR}")

# include source files
file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/src/*.ipp ${CMAKE_CURRENT_SOURCE_DIR}/src/*.inl)
file(GLOB_RECURSE RESOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/res/*.rc)

if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -pedantic)
endif()

# Engine: Create library
add_library(${SubProject} STATIC ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})

# Extra include folders
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")

#miniaudio config
find_path(MINIAUDIO_INCLUDE_DIRS "miniaudio.h")
include_directories("${CMAKE_SOURCE_DIR}/src" "${MINIAUDIO_INCLUDE_DIRS}")

# Set project debug working dir
set(DEBUGGER_WD "${CMAKE_SOURCE_DIR}/${SubProject}_WD")
set_target_properties(${SubProject} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${DEBUGGER_WD})
message("  Debugger Work directory is: ${DEBUGGER_WD}")

# Link libraries
target_link_libraries(${SubProject} PRIVATE ${OPENGL_LIBRARIES})
target_link_libraries(${SubProject} PRIVATE glm::glm)
target_link_libraries(${SubProject} PRIVATE ${FFMPEG_LIBRARIES})
target_link_libraries(${SubProject} PRIVATE glad::glad)
target_link_libraries(${SubProject} PRIVATE glfw)
target_link_libraries(${SubProject} PRIVATE ws2_32)
target_link_libraries(${SubProject} PRIVATE assimp::assimp)
target_link_libraries(${SubProject} PRIVATE imgui::imgui)
target_link_libraries(${SubProject} PRIVATE kissfft::kissfft-float)

install(
	TARGETS ${SubProject}
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	)

create_msvc_groups()
