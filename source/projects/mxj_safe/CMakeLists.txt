cmake_minimum_required(VERSION 3.16)

if (NOT APPLE)
	message(STATUS "mxj-safe won't be built on this platform.")
	return()
endif ()

set(C74_BUILD_MAX_EXTENSION TRUE)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-pretarget.cmake)

if (APPLE)
	if (CMAKE_OSX_ARCHITECTURES STREQUAL "" 
		OR CMAKE_OSX_ARCHITECTURES STREQUAL "i386")
		set(CMAKE_OSX_ARCHITECTURES x86_64) # override the old max-api values permitting i386
	endif ()
	if (CMAKE_OSX_DEPLOYMENT_TARGET STREQUAL "")
		set(CMAKE_OSX_DEPLOYMENT_TARGET 10.9)
	endif ()
endif ()

include_directories(
	"${MAX_SDK_INCLUDES}"
	"${C74_SUPPORT_DIR}"
)

add_library(
	${PROJECT_NAME}
	MODULE
	${PROJECT_NAME}.cpp
)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-posttarget.cmake)
