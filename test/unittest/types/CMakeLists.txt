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

# XRCE test
set(SRCS
    XRCETests.cpp
    ../Common.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/types/XRCETypes.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/types/MessageHeader.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/types/SubMessageHeader.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/message/OutputMessage.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/message/InputMessage.cpp
    )

add_executable(test-xrce-types ${SRCS})

add_gtest(test-xrce-types
    SOURCES
        ${SRCS}
    DEPENDENCIES
        fastcdr
    )

target_include_directories(test-xrce-types
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_BINARY_DIR}/include
        ${GTEST_INCLUDE_DIRS}
    )

target_link_libraries(test-xrce-types
    PRIVATE
        fastcdr
        $<$<BOOL:${UAGENT_LOGGER_PROFILE}>:spdlog::spdlog>
        ${GTEST_BOTH_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
    )

set_target_properties(test-xrce-types PROPERTIES
    CXX_STANDARD
        11
    CXX_STANDARD_REQUIRED
        YES
    )
