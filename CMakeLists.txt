cmake_minimum_required(VERSION 3.0)

PROJECT(max-mxj)

if (APPLE)
	message(STATUS "Building mxj.mxo")
	execute_process(
		COMMAND				xcodebuild -project mxj.xcodeproj
		WORKING_DIRECTORY	"${CMAKE_CURRENT_SOURCE_DIR}/source/mxj"
	)
	message(STATUS "Building mxj~.mxo")
	execute_process(
		COMMAND				xcodebuild -project mxj~.xcodeproj
		WORKING_DIRECTORY	"${CMAKE_CURRENT_SOURCE_DIR}/source/mxj"
	)
else ()
	set (MSBUILD "C:\\Program Files (x86)\\MSBuild\\12.0\\Bin\\MSBuild.exe")
	
	message(STATUS "Building mxj 64")
	execute_process(
		COMMAND				"${MSBUILD}" /target:rebuild /p:Platform=x64 mxj.vcxproj
		WORKING_DIRECTORY	"${CMAKE_CURRENT_SOURCE_DIR}/source/mxj"
	)

	message(STATUS "Building mxj~ 64")
	execute_process(
		COMMAND				"${MSBUILD}" /target:rebuild /p:Platform=x64 mxj~.vcxproj
		WORKING_DIRECTORY	"${CMAKE_CURRENT_SOURCE_DIR}/source/mxj"
	)
endif ()


if (APPLE)
	
	######################################################################
	message(STATUS "NOW WORKING ON THE SAFE WRAPPERS")
	######################################################################


	# Fetch the correct verion of the max-sdk
	message(STATUS "Updating Git Submodules")
	execute_process(
		COMMAND				git submodule update --init --recursive
		WORKING_DIRECTORY	"${CMAKE_CURRENT_SOURCE_DIR}"
	)


	# Misc setup and subroutines
	include(${CMAKE_CURRENT_SOURCE_DIR}/source/max-sdk/source/script/max-package.cmake)


	# Generate a project for every folder in the "source/projects" folder
	SUBDIRLIST(PROJECT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/source/projects)
	foreach (project_dir ${PROJECT_DIRS})
		if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/source/projects/${project_dir}/CMakeLists.txt")
			message("Generating: ${project_dir}")
			add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/${project_dir})
		endif ()
	endforeach ()
	
endif() # APPLE