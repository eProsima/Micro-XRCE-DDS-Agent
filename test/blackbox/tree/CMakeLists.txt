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
        TreeTests.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/types/XRCETypes.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/types/MessageHeader.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/types/SubMessageHeader.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/Root.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/participant/Participant.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/topic/Topic.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/client/ProxyClient.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/publisher/Publisher.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/subscriber/Subscriber.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/datawriter/DataWriter.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/datareader/DataReader.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/requester/Requester.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/replier/Replier.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/object/XRCEObject.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/types/TopicPubSubType.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/middleware/fastdds/FastDDSEntities.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/middleware/fastdds/FastDDSMiddleware.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/middleware/ced/CedEntities.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/middleware/ced/CedMiddleware.cpp
        )
    add_executable(tree_test ${SRCS})

    add_gtest(tree_test
        SOURCES
            ${SRCS}
        DEPENDENCIES
            fastdds
            fastcdr
        )
    target_include_directories(tree_test PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_BINARY_DIR}/include
        ${GTEST_INCLUDE_DIRS}
        ${GMOCK_INCLUDE_DIRS}
        ${ASIO_INCLUDE_DIR}
        )
    target_link_libraries(tree_test
        PRIVATE
            fastdds
            fastcdr
            $<$<BOOL:${UAGENT_LOGGER_PROFILE}>:spdlog::spdlog>
            ${GTEST_LIBRARIES}
            ${GMOCK_LIBRARIES}
        )
    target_compile_definitions(tree_test
        PRIVATE
            -DBOOST_ASIO_STANDALONE
            -DASIO_STANDALONE
        )
    set_target_properties(tree_test PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        )
    file(COPY ${PROJECT_SOURCE_DIR}/test/agent.refs
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
        )
endif()
