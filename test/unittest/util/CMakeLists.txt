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

# Utils test
set(SRCS
    UtilsTests.cpp
    ../Common.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/types/XRCETypes.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/types/MessageHeader.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/types/SubMessageHeader.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/datareader/TokenBucket.cpp
    )
add_executable(util_test ${SRCS})
add_gtest(util_test
    SOURCES ${SRCS}
    )
target_include_directories(util_test PRIVATE ${PROJECT_SOURCE_DIR}/include ${GTEST_INCLUDE_DIRS})
target_link_libraries(util_test PRIVATE fastcdr ${GTEST_BOTH_LIBRARIES})
set_target_properties(util_test PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    )
