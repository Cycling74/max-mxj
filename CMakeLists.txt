cmake_minimum_required(VERSION 3.16)

if (APPLE)
	set(CMAKE_OSX_ARCHITECTURES "arm64")
	set(CMAKE_OSX_DEPLOYMENT_TARGET 10.9)
else ()
	set(CMAKE_SYSTEM_VERSION 6.1)
	# Windows 7   => 6.1
	# Windows 8   => 6.2
	# Windows 8.1 => 6.3
	# Windows 10  => 10.0
endif ()

project(max-mxj C CXX)

message(STATUS "Updating Git Submodules")
execute_process(
	COMMAND				git submodule update --init --recursive
	WORKING_DIRECTORY	"${CMAKE_CURRENT_SOURCE_DIR}"
)

message(STATUS "Iterating projects...")

# Misc setup and subroutines
include(${CMAKE_CURRENT_SOURCE_DIR}/source/max-api/script/max-package.cmake)

# Generate a project for every folder in the "source/projects" folder
SUBDIRLIST(PROJECT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/source/projects)
foreach (project_dir ${PROJECT_DIRS})
	if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/source/projects/${project_dir}/CMakeLists.txt")
		message("Generating: ${project_dir}")
		add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/${project_dir})
	endif ()
endforeach ()
