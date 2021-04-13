cmake_minimum_required(VERSION 3.16)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-api/script/max-pretarget.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../mxj-common.cmake)

if (APPLE)
	set(CMAKE_OSX_ARCHITECTURES arm64) # override the old max-api values permitting i386
endif ()

add_library(
	${PROJECT_NAME}
	MODULE
	${SOURCES}
	${HEADERS}
)

target_precompile_headers(${PROJECT_NAME} PRIVATE "${SRCDIR}/mxj_macho_prefix.h")

include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-api/script/max-posttarget.cmake)
