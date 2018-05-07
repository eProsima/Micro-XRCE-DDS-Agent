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

add_subdirectory(integration/cross_serialization)
add_subdirectory(integration/xrce_serialization)

include(${PROJECT_SOURCE_DIR}/cmake/dev/gtest.cmake)
check_gtest()
if(GTEST_FOUND)

    # Utils test
    add_executable(util_test_exec UtilsTests.cpp Common.cpp)
    add_test(NAME util_test COMMAND util_test_exec)
    target_include_directories(util_test_exec PRIVATE ${PROJECT_SOURCE_DIR} ${GTEST_INCLUDE_DIRS})
    target_link_libraries(util_test_exec PRIVATE micrortps-agent ${GTEST_BOTH_LIBRARIES})
    set_target_properties(util_test_exec PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        )
    if(WIN32)
        set(DEPENDENCIES
            micrortps-agent
            )
        set(WIN_PATH "$ENV{PATH}")
        foreach(P ${CMAKE_PREFIX_PATH})
            set(WIN_PATH "${CMAKE_PREFIX_PATH}/bin;${WIN_PATH}")
        endforeach()
        foreach(D ${DEPENDENCIES})
            if(TARGET ${D})
                set(WIN_PATH "$<TARGET_FILE_DIR:${D}>;${WIN_PATH}")
            endif()
        endforeach()
        string(REPLACE ";" "\\;" WIN_PATH "${WIN_PATH}")
        set_tests_properties(util_test PROPERTIES ENVIRONMENT "PATH=${WIN_PATH}")
    endif()

    # XRCE test
    add_executable(xrce_test_exec XRCETests.cpp Common.cpp)
    add_test(NAME xrce_test COMMAND xrce_test_exec)
    target_include_directories(xrce_test_exec PRIVATE ${PROJECT_SOURCE_DIR} ${GTEST_INCLUDE_DIRS})
    target_link_libraries(xrce_test_exec PRIVATE micrortps-agent ${GTEST_BOTH_LIBRARIES})
    set_target_properties(xrce_test_exec PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        )
    if(WIN32)
        set(DEPENDENCIES
            micrortps-agent
            fastcdr
            )
        set(WIN_PATH "$ENV{PATH}")
        foreach(P ${CMAKE_PREFIX_PATH})
            set(WIN_PATH "${CMAKE_PREFIX_PATH}/bin;${WIN_PATH}")
        endforeach()
        foreach(D ${DEPENDENCIES})
            if(TARGET ${D})
                set(WIN_PATH "$<TARGET_FILE_DIR:${D}>;${WIN_PATH}")
            endif()
        endforeach()
        string(REPLACE ";" "\\;" WIN_PATH "${WIN_PATH}")
        set_tests_properties(xrce_test PROPERTIES ENVIRONMENT "PATH=${WIN_PATH}")
    endif()

    # Agent test
    file(COPY ${PROJECT_SOURCE_DIR}/DEFAULT_FASTRTPS_PROFILES.xml
        DESTINATION ${PROJECT_BINARY_DIR}/test
    )

    add_executable(agent_test_exec
        AgentTests.cpp
        DataReaderTests.cpp
        Common.cpp
        )
    add_test(NAME agent_test COMMAND agent_test_exec)
    target_include_directories(agent_test_exec PRIVATE ${PROJECT_SOURCE_DIR} ${GTEST_INCLUDE_DIRS})
    target_link_libraries(agent_test_exec micrortps-agent ${GTEST_BOTH_LIBRARIES})
    set_target_properties(agent_test_exec PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        )
    if(WIN32)
        set(DEPENDENCIES
            micrortps-agent
            transport
            fastrtps
            fastcdr
            )
        set(WIN_PATH "$ENV{PATH}")
        foreach(P ${CMAKE_PREFIX_PATH})
            set(WIN_PATH "${CMAKE_PREFIX_PATH}/bin;${WIN_PATH}")
        endforeach()
        foreach(D ${DEPENDENCIES})
            if(TARGET ${D})
                set(WIN_PATH "$<TARGET_FILE_DIR:${D}>;${WIN_PATH}")
            endif()
        endforeach()
        string(REPLACE ";" "\\;" WIN_PATH "${WIN_PATH}")
        set_tests_properties(agent_test PROPERTIES ENVIRONMENT "PATH=${WIN_PATH}")
    endif()

endif()
