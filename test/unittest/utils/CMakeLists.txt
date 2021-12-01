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

###################################################################################################
# TokenBucketTest
###################################################################################################

set(SRCS
    TokenBucketTest.cpp
    )

add_executable(test-token-bucket ${SRCS})

add_gtest(test-token-bucket
    SOURCES
        ${SRCS}
    )

target_include_directories(test-token-bucket
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${GTEST_INCLUDE_DIRS}
    )

target_link_libraries(test-token-bucket
    PRIVATE
        ${GTEST_BOTH_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
    )

set_target_properties(test-token-bucket PROPERTIES
    CXX_STANDARD
        11
    CXX_STANDARD_REQUIRED
        YES
    )

###################################################################################################
# SeqNumTest
###################################################################################################

set(SRCS
    SeqNumUnitTests.cpp
    )
add_executable(test-seq-num ${SRCS})
add_gtest(test-seq-num
    SOURCES
        ${SRCS}
    DEPENDENCIES
        fastcdr
    )
target_include_directories(test-seq-num PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    ${GTEST_INCLUDE_DIRS}
    ${GMOCK_INCLUDE_DIRS}
    )
target_link_libraries(test-seq-num
    PRIVATE
        fastcdr
        ${GTEST_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
    )
set_target_properties(test-seq-num PROPERTIES
    CXX_STANDARD
        11
    CXX_STANDARD_REQUIRED
        YES
    )
