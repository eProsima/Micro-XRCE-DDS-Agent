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
# OutputStreamTest
###################################################################################################

set(SRCS
    OutputStreamTest.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/types/XRCETypes.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/types/MessageHeader.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/types/SubMessageHeader.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/message/OutputMessage.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/message/InputMessage.cpp
    )

add_executable(test-output-stream ${SRCS})

add_gtest(test-output-stream
    SOURCES
        ${SRCS}
    DEPENDENCIES
        fastcdr
    )

target_include_directories(test-output-stream
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_BINARY_DIR}/include
        ${GTEST_INCLUDE_DIRS}
        ${GMOCK_INCLUDE_DIRS}
    )

target_link_libraries(test-output-stream
    PRIVATE
        fastcdr
        $<$<BOOL:${UAGENT_LOGGER_PROFILE}>:spdlog::spdlog>
        ${GTEST_BOTH_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
    )

set_target_properties(test-output-stream PROPERTIES
    CXX_STANDARD
        11
    CXX_STANDARD_REQUIRED
        YES
    )

###################################################################################################
# InputStreamTest
###################################################################################################

set(SRCS
    InputStreamTest.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/types/XRCETypes.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/types/MessageHeader.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/types/SubMessageHeader.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/message/OutputMessage.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/message/InputMessage.cpp
    )

add_executable(test-input-stream ${SRCS})

add_gtest(test-input-stream
    SOURCES
        ${SRCS}
    DEPENDENCIES
        fastcdr
    )

target_include_directories(test-input-stream PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    ${GTEST_INCLUDE_DIRS}
    ${GMOCK_INCLUDE_DIRS}
    )

target_link_libraries(test-input-stream
    PRIVATE
        fastcdr
        $<$<BOOL:${UAGENT_LOGGER_PROFILE}>:spdlog::spdlog>
        ${GTEST_BOTH_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
    )

set_target_properties(test-input-stream PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    )
