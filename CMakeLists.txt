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

project(micrortps_agent)

###############################################################################
# eProsima build options
###############################################################################
option(EPROSIMA_BUILD "Activate internal building" OFF)

###############################################################################
# Load external eProsima projects.
###############################################################################
include(${PROJECT_SOURCE_DIR}/cmake/dev/eprosima_libraries.cmake)
option(THIRDPARTY "Activate the build of thirdparties" OFF)
option(UPDATE_SUBMODULES "Update to the latest" OFF)
option(VERBOSE_OUTPUT "Use verbose output" OFF)

if(VERBOSE_OUTPUT)
    add_definitions(-DVERBOSE_OUTPUT)
endif()

if(EPROSIMA_BUILD)
    set(THIRDPARTY ON)
endif()

###############################################################################
# Compile
###############################################################################
# Find requirements
find_package(fastrtps REQUIRED)
find_package(fastcdr REQUIRED)
find_eprosima_package(micrortps_transport)

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

# Required libs for examples
include_directories(thirdparty/micrortps_transport/include)
if(NOT TARGET transport)
     add_subdirectory(thirdparty/micrortps_transport)
endif()

include_directories(${CMAKE_PREFIX_PATH}/include
                    src/cpp
                    include
                    include/agent
                    #/home/vmonge/Workspace/Fast-RTPS/thirdparty/asio/asio/include
                    /home/borja/dev/rtps/thirdparty/asio/asio/include
                    #/home/luis/eProsima/Fast-RTPS/thirdparty/asio/asio/include
                    )

# libagent.so
file(GLOB AGENT_LIB_SRCS "src/cpp/*.cpp" "src/cpp/*.cxx" "src/cpp/*/*.cpp" "src/cpp/*/*.cxx")
add_library(agent SHARED ${AGENT_LIB_SRCS})
install(TARGETS agent DESTINATION ${CMAKE_SOURCE_DIR}/install)
target_link_libraries(agent transport fastcdr)

# agent_parser_test
file(GLOB AGENT_PARSER_TEST_SOURCES "agent_parser_test.cpp")
add_executable(agent_parser_test ${AGENT_PARSER_TEST_SOURCES})
install(TARGETS agent_parser_test DESTINATION ${CMAKE_SOURCE_DIR}/install)
target_link_libraries(agent_parser_test transport agent fastcdr fastrtps)

# micrortps_agent
file(GLOB MICRORTPS_AGENT_SOURCES "micrortps_agent.cpp")
add_executable(micrortps_agent ${MICRORTPS_AGENT_SOURCES})
install(TARGETS micrortps_agent DESTINATION ${CMAKE_SOURCE_DIR}/install)
target_link_libraries(micrortps_agent transport agent fastcdr fastrtps)

# shape_subs_test
file(GLOB SHAPE_TEST_SOURCES "shape_subs_test.cpp")
add_executable(shape_subs_test ${SHAPE_TEST_SOURCES})
install(TARGETS shape_subs_test DESTINATION ${CMAKE_SOURCE_DIR}/install)
target_link_libraries(shape_subs_test agent fastcdr fastrtps)

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
