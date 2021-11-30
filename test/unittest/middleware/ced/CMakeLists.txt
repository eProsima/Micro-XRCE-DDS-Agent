# Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

set(TEST_NAME "ced-middleware-unit-tests")

set(SRCS
    CedMiddlewareTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/middleware/ced/CedMiddleware.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/middleware/ced/CedEntities.cpp
    )

add_executable(${TEST_NAME} ${SRCS})

add_gtest(${TEST_NAME} SOURCES ${SRCS})

target_include_directories(${TEST_NAME}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_BINARY_DIR}/include
        ${GTEST_INCLUDE_DIRS}
        ${GMOCK_INCLUDE_DIRS}
    )

target_link_libraries(${TEST_NAME}
    PRIVATE
        ${GTEST_BOTH_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
    )

set_target_properties(${TEST_NAME} PROPERTIES
    CXX_STANDARD
        11
    CXX_STANDARD_REQUIRED
        YES
    )
