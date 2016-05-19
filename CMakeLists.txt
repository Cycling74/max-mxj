cmake_minimum_required(VERSION 3.0)

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
	
	message(STATUS "Building mxj 32")
	execute_process(
		COMMAND				"${MSBUILD}" /target:rebuild /p:Platform=Win32 mxj.vcxproj
		WORKING_DIRECTORY	"${CMAKE_CURRENT_SOURCE_DIR}/source/mxj"
	)
	message(STATUS "Building mxj 64")
	execute_process(
		COMMAND				"${MSBUILD}" /target:rebuild /p:Platform=x64 mxj.vcxproj
		WORKING_DIRECTORY	"${CMAKE_CURRENT_SOURCE_DIR}/source/mxj"
	)

	message(STATUS "Building mxj~ 32")
	execute_process(
		COMMAND				"${MSBUILD}" /target:rebuild /p:Platform=Win32 mxj~.vcxproj
		WORKING_DIRECTORY	"${CMAKE_CURRENT_SOURCE_DIR}/source/mxj"
	)
	message(STATUS "Building mxj~ 64")
	execute_process(
		COMMAND				"${MSBUILD}" /target:rebuild /p:Platform=x64 mxj~.vcxproj
		WORKING_DIRECTORY	"${CMAKE_CURRENT_SOURCE_DIR}/source/mxj"
	)
endif ()
