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

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

if(CMAKE_VERSION VERSION_LESS 3.0)
    project(micrortps-agent_tests C CXX)
else()
    cmake_policy(SET CMP0048 NEW)
    project(micrortps-agent_tests LANGUAGES C CXX)
endif()

include(${PROJECT_SOURCE_DIR}/cmake/dev/gtest.cmake)
check_gtest()
if(GTEST_FOUND)

    # Utils test
    add_executable(util_test UtilsTests.cpp Common.cpp)
    add_gtest(util_test UtilsTests.cpp Common.cpp)
    target_include_directories(util_test PRIVATE ${PROJECT_SOURCE_DIR} ${GTEST_INCLUDE_DIRS})
    target_link_libraries(util_test PRIVATE micrortps-agent ${GTEST_BOTH_LIBRARIES})
    set_target_properties(util_test PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        )

    # XRCE test
    add_executable(xrce_test XRCETests.cpp Common.cpp)
    add_gtest(xrce_test XRCETests.cpp Common.cpp)
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

    add_executable(agent_test
        AgentTests.cpp
        DataReaderTests.cpp
        Common.cpp
        )
    add_gtest(agent_test
        AgentTests.cpp
        DataReaderTests.cpp
        Common.cpp
        )
    target_include_directories(agent_test PRIVATE ${PROJECT_SOURCE_DIR} ${GTEST_INCLUDE_DIRS})
    target_link_libraries(agent_test micrortps-agent ${GTEST_BOTH_LIBRARIES})
    set_target_properties(agent_test PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        )

    if(WIN32)
        string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
        set_property(TEST AgentTest APPEND PROPERTY ENVIRONMENT
            "PATH=$<SHELL_PATH:$<TARGET_FILE_DIR:transport>\\;$<TARGET_FILE_DIR:fastrtps>\\;
            $<TARGET_FILE_DIR:fastcdr>>\\;${WIN_PATH}"
            )
    endif()

endif()
