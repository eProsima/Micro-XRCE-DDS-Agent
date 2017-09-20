################################################################################
#
# Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors
# may be used to endorse or promote products derived from this software without
# specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
################################################################################

cmake_minimum_required(VERSION 2.8.12)
project(PoC_SendReceive)

find_package(fastrtps REQUIRED)
find_package(fastcdr REQUIRED)

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
                    transport/include
                    transport/include/transport
                    agent/include
                    agent/src/cpp
                    agent/include/agent
                    client/include
                    client/include/client)

# libtransport.so
file(GLOB TRANS_LIB_SRCS "transport/src/c/transport/*.c")
add_library(transport SHARED ${TRANS_LIB_SRCS})
install(TARGETS transport DESTINATION ${CMAKE_SOURCE_DIR}/install)

# libagent.so
file(GLOB AGENT_LIB_SRCS "agent/src/cpp/*.cpp" "agent/src/cpp/*/*.cpp" "agent/src/cpp/*/*.cxx")
add_library(agent SHARED ${AGENT_LIB_SRCS})
install(TARGETS agent DESTINATION ${CMAKE_SOURCE_DIR}/install)

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

# libclient.so
file(GLOB CLIENT_LIB_SRCS "client/src/C/*.c" "client/src/C/*/*.c")
add_library(client SHARED ${CLIENT_LIB_SRCS})
install(TARGETS client DESTINATION ${CMAKE_SOURCE_DIR}/install)
#install(FILES client/include/*.h DESTINATION ${CMAKE_SOURCE_DIR}/install/include/client/)

# client_test
# file(GLOB CLIENT_TEST_SOURCES "client_test.c")
# add_executable(client_test ${CLIENT_TEST_SOURCES})
# install(TARGETS client_test DESTINATION ${CMAKE_SOURCE_DIR}/install)
# target_link_libraries(client_test client transport)

# # shape_subs_test
# file(GLOB SHAPE_TEST_SOURCES "shape_subs_test.cpp")
# add_executable(shape_subs_test ${SHAPE_TEST_SOURCES})
# install(TARGETS shape_subs_test DESTINATION ${CMAKE_SOURCE_DIR}/install)
# target_link_libraries(shape_subs_test agent fastcdr fastrtps)

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
