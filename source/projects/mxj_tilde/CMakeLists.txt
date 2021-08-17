cmake_minimum_required(VERSION 3.16)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk/source/max-sdk-base/script/max-pretarget.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../mxj-common.cmake)

add_library(
	${PROJECT_NAME}
	MODULE
	${SOURCES}
	${HEADERS}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if (NOT APPLE)
	target_precompile_headers(${PROJECT_NAME} PRIVATE "${SRCDIR}/mxj_macho_prefix.h")
endif ()

target_compile_definitions(${PROJECT_NAME} PUBLIC MXJ_MSP=1)
if (APPLE)
	set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_PRODUCT_NAME "mxj~")
endif ()

include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk/source/max-sdk-base/script/max-posttarget.cmake)
