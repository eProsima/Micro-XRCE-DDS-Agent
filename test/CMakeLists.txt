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

include(${PROJECT_SOURCE_DIR}/test/cmake/dev/gtest.cmake)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

if(CMAKE_VERSION VERSION_LESS 3.0)
    project(micrortps-agent_tests C CXX)
else()
    cmake_policy(SET CMP0048 NEW)
    project(micrortps-agent_tests LANGUAGES C CXX)
endif()

add_subdirectory(integration/interaction)
add_subdirectory(integration/cross_serialization)
add_subdirectory(integration/xrce_serialization)

check_gtest()
if(GTEST_FOUND)
    include(CTest)

    # Utils test
    set(SRCS
        UtilsTests.cpp
        Common.cpp
        )
    add_executable(util_test ${SRCS})
    add_gtest(util_test
        SOURCES
            ${SRCS}
        DEPENDENCIES
            micrortps-agent
        )
    target_include_directories(util_test PRIVATE ${PROJECT_SOURCE_DIR} ${GTEST_INCLUDE_DIRS})
    target_link_libraries(util_test PRIVATE micrortps-agent ${GTEST_BOTH_LIBRARIES})
    set_target_properties(util_test PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        )

    # XRCE test
    set(SRCS
        XRCETests.cpp
        Common.cpp
        )
    add_executable(xrce_test ${SRCS})
    add_gtest(xrce_test
        SOURCES
            ${SRCS}
        DEPENDENCIES
            micrortps-agent
            fastrtps
        )
    target_include_directories(xrce_test PRIVATE ${PROJECT_SOURCE_DIR} ${GTEST_INCLUDE_DIRS})
    target_link_libraries(xrce_test PRIVATE micrortps-agent ${GTEST_BOTH_LIBRARIES})
    set_target_properties(xrce_test PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        )

    # Agent test
    file(COPY ${PROJECT_SOURCE_DIR}/DEFAULT_FASTRTPS_PROFILES.xml
        DESTINATION ${PROJECT_BINARY_DIR}/test
    )

    set(SRCS
        AgentTests.cpp
        DataReaderTests.cpp
        Common.cpp
        )
    add_executable(agent_test ${SRCS})
    add_gtest(agent_test
        SOURCES
            ${SRCS}
        DEPENDENCIES
            micrortps-agent
            micrortps-transport
            fastrtps
            fastcdr
        )
    target_include_directories(agent_test PRIVATE ${PROJECT_SOURCE_DIR} ${GTEST_INCLUDE_DIRS})
    target_link_libraries(agent_test micrortps-agent ${GTEST_BOTH_LIBRARIES})
    set_target_properties(agent_test PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        )
endif()
