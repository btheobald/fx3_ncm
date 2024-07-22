cmake_minimum_required(VERSION 3.10)

set (CMAKE_CONFIGURATION_TYPES "Debug;Release")

project(lwIP)

# Example lwIP application
set(LWIP_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Generate docs before creating source package
include(src/Filelists.cmake)

add_library(lwip ${lwipnoapps_SRCS} ${lwipiperf_SRCS})
target_compile_options(lwip PRIVATE ${LWIP_COMPILER_FLAGS})
target_compile_definitions(lwip PRIVATE ${LWIP_DEFINITIONS}  ${LWIP_MBEDTLS_DEFINITIONS})
target_include_directories(lwip PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src/include ${CMAKE_CURRENT_LIST_DIR})