# Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

###############################################################################
# CMake build rules for micro RTPS agent
###############################################################################
cmake_minimum_required(VERSION 2.8.12)

# Set CMAKE_BUILD_TYPE to Release by default.
if(DEFINED CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
    )
else()
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
        )
endif()

# TODO remove C language will crash transport
project(micrortps_agent)

# Set product name
if(DEFINED PRODUCT_NAME)
    set(PRODUCT_NAME ${PRODUCT_NAME} CACHE STRING "Choose the product name in which this project is included")
else()
    set(PRODUCT_NAME micrortps CACHE STRING "Choose the product name in which this project is included")
endif()

###############################################################################
# Version information                                                         #
###############################################################################
set(PROJECT_MAJOR_VERSION 0)
set(PROJECT_MINOR_VERSION 4)
set(PROJECT_MICRO_VERSION 0)
set(PROJECT_VERSION ${PROJECT_MAJOR_VERSION}.${PROJECT_MINOR_VERSION}.${PROJECT_MICRO_VERSION})
message(STATUS "${PROJECT_NAME} version: ${PROJECT_VERSION}")

###############################################################################
# Installation paths
###############################################################################
set(BIN_INSTALL_DIR bin/ CACHE PATH "Installation directory for binaries")
set(INCLUDE_INSTALL_DIR include/ CACHE PATH "Installation directory for C++ headers")
set(LIB_INSTALL_DIR lib/ CACHE PATH "Installation directory for libraries")
set(DATA_INSTALL_DIR share/ CACHE PATH "Installation directory for data")
if(WIN32)
    set(DOC_DIR "doc")
else()
    set(DOC_DIR "${DATA_INSTALL_DIR}/doc")
endif()
set(DOC_INSTALL_DIR ${DOC_DIR} CACHE PATH "Installation directory for documentation")
if(WIN32)
    set(LICENSE_INSTALL_DIR . CACHE PATH "Installation directory for licenses")
else()
    set(LICENSE_INSTALL_DIR ${DATA_INSTALL_DIR}/${PROJECT_NAME} CACHE PATH "Installation directory for licenses")
endif()

###############################################################################
# eProsima build options
###############################################################################
option(EPROSIMA_BUILD "Activate internal building" OFF)

###############################################################################
# Load CMake modules
###############################################################################
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/modules)

###############################################################################
# Test system configuration
###############################################################################
include(${PROJECT_SOURCE_DIR}/cmake/dev/check_configuration.cmake)
option(CHECK_ENDIANNESS "Enable/disable endiannes checking" ON)
if(CHECK_ENDIANNESS)
    check_endianness()
else()
    message(STATUS "WARNING!! micrortps-client without check endianness")
endif()

###############################################################################
# Load external eProsima projects.
###############################################################################
include(${PROJECT_SOURCE_DIR}/cmake/dev/eprosima_libraries.cmake)
option(THIRDPARTY "Activate the build of thirdparties" OFF)
option(UPDATE_SUBMODULES "Update to the latest" OFF)
option(VERBOSE_OUTPUT "Use verbose output" ON)

if(VERBOSE_OUTPUT)
    add_definitions(-DVERBOSE_OUTPUT)
endif()

if(EPROSIMA_BUILD)
    set(THIRDPARTY ON)
endif()

add_definitions(
    -DBOOST_ASIO_STANDALONE
    -DASIO_STANDALONE
  )

###############################################################################
# Compile
###############################################################################
# Find requirements
###############################################################################
# Load external eProsima projects.
###############################################################################
include(${PROJECT_SOURCE_DIR}/cmake/dev/eprosima_libraries.cmake)
find_package(fastrtps REQUIRED)
find_package(fastcdr REQUIRED)
find_eprosima_thirdparty(Asio asio)
if(NOT TARGET transport)
  add_subdirectory(thirdparty/micrortps_transport)
endif()

# Set C++11
include(CheckCXXCompilerFlag)
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG OR
    CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    check_cxx_compiler_flag(--std=c++11 SUPPORTS_CXX11)
    if(SUPPORTS_CXX11)
        add_compile_options(--std=c++11)
    else()
        message(FATAL_ERROR "Compiler doesn't support C++11")
    endif()
endif()

include_directories(${CMAKE_PREFIX_PATH}/include
                    src/cpp
                    include
                    include/agent
                    ${ASIO_INCLUDE_DIR}
                    )

# libagent.so
file(GLOB AGENT_LIB_SRCS "src/cpp/*.cpp" "src/cpp/*.cxx" "src/cpp/*/*.cpp" "src/cpp/*/*.cxx")
add_library(agent STATIC ${AGENT_LIB_SRCS})
target_link_libraries(agent transport fastcdr fastrtps)
#TODO: fix it, import as public only the necesary paths and headers
target_include_directories(
    agent PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/cpp>
    $<INSTALL_INTERFACE:include>
)

# micrortps_agent
file(COPY ${PROJECT_SOURCE_DIR}/DEFAULT_FASTRTPS_PROFILES.xml
DESTINATION ${PROJECT_BINARY_DIR})

file(GLOB MICRORTPS_AGENT_SOURCES "micrortps_agent.cpp")
add_executable(${PROJECT_NAME} ${MICRORTPS_AGENT_SOURCES})
target_link_libraries(${PROJECT_NAME} transport agent fastcdr fastrtps)

###############################################################################
# Warning level
###############################################################################
if(MSVC OR MSVC_IDE)
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()

    if(EPROSIMA_EXTRA_CMAKE_CXX_FLAGS)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EPROSIMA_EXTRA_CMAKE_CXX_FLAGS}")
    endif()
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -Wno-unknown-pragmas -std=c++11")
    # -fprofile-arcs -ftest-coverage")
endif()

###############################################################################
# Testing
###############################################################################
option(EPROSIMA_BUILD_TESTS "Activate the building and execution unit tests and integral tests" OFF)
if(EPROSIMA_BUILD_TESTS)
    include(${PROJECT_SOURCE_DIR}/cmake/dev/gtest.cmake)
    check_gtest()
    if(GTEST_FOUND)
        enable_testing()
        include(CTest)
        add_subdirectory(test)
    endif()
endif()

###############################################################################
# Packaging
###############################################################################
install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets
        RUNTIME DESTINATION ${BIN_INSTALL_DIR}
        LIBRARY DESTINATION ${LIB_INSTALL_DIR}
        ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
        COMPONENT libraries
)

export(TARGETS ${PROJECT_NAME} FILE ${PROJECT_BINARY_DIR}/cmake/config/${PROJECT_NAME}Targets.cmake)

install(EXPORT ${PROJECT_NAME}Targets
DESTINATION ${LIB_INSTALL_DIR}/${PROJECT_NAME}/cmake
COMPONENT cmake
)
include(${PROJECT_SOURCE_DIR}/cmake/packaging/eProsimaPackaging.cmake)
