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

if(GMOCK_FOUND)
    set(SRCS
        RootUnitTests.cpp
        ../Common.cpp
        )
    add_executable(root_unit_test ${SRCS})
    add_gtest(root_unit_test
        SOURCES
            ${SRCS}
        DEPENDENCIES
            micrortps_agent
            fastcdr
            fastrtps
        )
    target_include_directories(root_unit_test PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_BINARY_DIR}/include
        ${GTEST_INCLUDE_DIRS}
        ${GMOCK_INCLUDE_DIRS}
        )
    target_link_libraries(root_unit_test PRIVATE micrortps_agent ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES})
    set_target_properties(root_unit_test PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        )
    file(COPY ${PROJECT_SOURCE_DIR}/DEFAULT_FASTRTPS_PROFILES.xml
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
        )
endif()
