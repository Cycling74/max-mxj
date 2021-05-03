cmake_minimum_required(VERSION 3.16)

if (NOT APPLE)
	message(STATUS "mxj-safe won't be built on this platform.")
	return()
endif ()

set(C74_BUILD_MAX_EXTENSION TRUE)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-api/script/max-pretarget.cmake)

if (APPLE)
	set(CMAKE_OSX_ARCHITECTURES arm64) # override the old max-api values permitting i386
endif ()

include_directories(
	"${C74_INCLUDES}"
)

add_library(
	${PROJECT_NAME}
	MODULE
	${PROJECT_NAME}.cpp
)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-api/script/max-posttarget.cmake)
