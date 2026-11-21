cmake_minimum_required(VERSION 3.21)
project(DLGen VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Geode's build action provides the SDK path as an environment variable.
# Also allow local builds to pass -DGEODE_SDK=... explicitly.
if (NOT DEFINED GEODE_SDK)
    if (DEFINED ENV{GEODE_SDK})
        set(GEODE_SDK "$ENV{GEODE_SDK}")
    else()
        message(FATAL_ERROR "GEODE_SDK is not set. Pass -DGEODE_SDK=/path/to/geode-sdk or set the GEODE_SDK environment variable.")
    endif()
endif()

add_subdirectory(${GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCES})
setup_geode_mod(${PROJECT_NAME} EXTERNALS geode:>=5.11.2)
