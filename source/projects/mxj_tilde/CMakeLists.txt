cmake_minimum_required(VERSION 3.16)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-api/script/max-pretarget.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../mxj-common.cmake)

if (APPLE)
	set(CMAKE_OSX_ARCHITECTURES x86_64) # override the old max-api values permitting i386
endif ()

add_library(
	${PROJECT_NAME}
	MODULE
	${SOURCES}
	${HEADERS}
)

target_precompile_headers(${PROJECT_NAME} PRIVATE "${SRCDIR}/mxj_macho_prefix.h")
target_compile_definitions(${PROJECT_NAME} PUBLIC MXJ_MSP=1)
if (APPLE)
	set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_PRODUCT_NAME "mxj~")
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-api/script/max-posttarget.cmake)
