cmake_minimum_required(VERSION 3.5)

set(FX3_SDK_DIR "/opt/ezusbfx3sdk/cyfx3sdk")
set(ENV{FX3_INSTALL_PATH} "/opt/ezusbfx3sdk/cyfx3sdk")
# Absolutely Ancient SDK compiler :(
#set(ENV{FX3_ARMGCC_INSTALL_PATH} "/opt/ezusbfx3sdk/arm-2013.11/")
#set(ENV{FX3_ARMGCC_VERSION} "4.8.1")
# New compiler seems to work?
set(ENV{FX3_ARMGCC_INSTALL_PATH} "/usr")
set(ENV{FX3_ARMGCC_VERSION} "14.1.0")
include(${CMAKE_SOURCE_DIR}/utl/cmake/fx3.cmake)

project(Fx3CmakeSample)

enable_language(ASM C CXX)

#
# Options
# =======
# Code validation and CI server friendly
set(WPEDANTIC OFF                              CACHE BOOL    "turn on/off -Wpedantic option for compiller (default: off)")
set(WERROR    OFF                              CACHE BOOL    "turn on/off -Werror option for compiller (default: off)")

# Project settings
#fx3_enable_cxx()
#fx3_disable_stdc_libs()
#fx3_enable_stdcxx_libs()

# Additional C++ flags
set(COMMON_FLAGS "-fmessage-length=0 -ffunction-sections -fdata-sections -fshort-wchar -Wall -Wextra")
if (WPEDANTIC)
  set(COMMON_FLAGS "${COMMON_FLAGS} -Wpedantic")
endif()
if (WERROR)
  set(COMMON_FLAGS "${COMMON_FLAGS} -Werror")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti -fno-unwind-tables -std=c++11 ${COMMON_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_FLAGS} -Wwrite-strings -std=c11")

add_subdirectory(lib)
add_subdirectory(src)
                                                
add_custom_target(flash_i2c ./build/download_fx3 -t I2C -i ${CMAKE_SOURCE_DIR}/build/${CMAKE_PROJECT_NAME}.img WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/utl/flash DEPENDS Fx3CmakeSample USES_TERMINAL)
add_custom_target(flash_ram ./build/download_fx3 -t RAM -i ${CMAKE_SOURCE_DIR}/build/${CMAKE_PROJECT_NAME}.img WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/utl/flash DEPENDS Fx3CmakeSample USES_TERMINAL)
