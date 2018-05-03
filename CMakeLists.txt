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

###############################################################################
# CMake build rules for Micro RTPS Agent
###############################################################################
cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

set(IS_TOP_LEVEL TRUE)
if(PROJECT_SOURCE_DIR)
    set(IS_TOP_LEVEL FALSE)
endif()

# Set CMAKE_BUILD_TYPE to Release by default.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

###############################################################################
# Product information
###############################################################################
if(CMAKE_VERSION VERSION_LESS 3.0)
    project(agent C CXX)
    set(PROJECT_VERSION_MAJOR 1)
    set(PROJECT_VERSION_MINOR 0)
    set(PROJECT_VERSION_PATCH 0)
    set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})
else()
    cmake_policy(SET CMP0048 NEW)
    project(agent VERSION "1.0.0" LANGUAGES C CXX)
endif()

# Set product name
if(DEFINED PRODUCT_NAME)
    set(PRODUCT_NAME ${PRODUCT_NAME} CACHE STRING "Choose the product name in which this project is included")
else()
    set(PRODUCT_NAME micrortps CACHE STRING "Choose the product name in which this project is included")
endif()

message(STATUS "${PROJECT_NAME} version: ${PROJECT_VERSION}")

###############################################################################
# eProsima build options
###############################################################################
option(EPROSIMA_BUILD_EXAMPLES "Compile examples" OFF)
option(EPROSIMA_BUILD_TESTS "Activate the building tests" OFF)
option(EPROSIMA_BUILD "Activate internal building" OFF)
option(THIRDPARTY "Activate the build of thirdparties" ON)
option(VERBOSE "Use verbose output" ON)

if(EPROSIMA_BUILD)
    set(THIRDPARTY ON)
    set(EPROSIMA_BUILD_TESTS ON)
endif()

###############################################################################
# Check MSVC architecture
###############################################################################
include(${PROJECT_SOURCE_DIR}/cmake/dev/check_configuration.cmake)
if(MSVC OR MSVC_IDE)
    check_msvc_arch()
endif()

###############################################################################
# Load external eProsima projects.
###############################################################################
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/modules)
include(${PROJECT_SOURCE_DIR}/cmake/dev/eprosima_libraries.cmake)
find_eprosima_package(fastcdr REQUIRED)
find_eprosima_package(fastrtps REQUIRED)
find_eprosima_thirdparty(Asio asio)
find_eprosima_package(micrortps_transport REQUIRED)

###############################################################################
# Targets
###############################################################################

# Set source files
set(SRCS
    src/cpp/Root.cpp
    src/cpp/XRCEObject.cpp
    src/cpp/XRCETypes.cpp
    src/cpp/XRCEFactory.cpp
    src/cpp/Serializer.cpp
    src/cpp/MessageHeader.cpp
    src/cpp/SubMessageHeader.cpp
    src/cpp/MessageQueue.cpp
    src/cpp/client/ProxyClient.cpp
    src/cpp/client/StreamsManager.cpp
    src/cpp/client/XRCEStreams.cpp
    src/cpp/participant/Participant.cpp
    src/cpp/topic/Topic.cpp
    src/cpp/publisher/Publisher.cpp
    src/cpp/subscriber/Subscriber.cpp
    src/cpp/datareader/DataReader.cpp
    src/cpp/datareader/TokenBucket.cpp
    src/cpp/datawriter/DataWriter.cpp
    src/cpp/types/TopicPubSubType.cpp
    src/cpp/xmlobjects/xmlobjects.cpp
    src/cpp/libdev/MessageOutput.cpp
    )

# Executable
add_executable(${PROJECT_NAME} micrortps_agent.cpp ${SRCS})
target_link_libraries(${PROJECT_NAME} PRIVATE transport fastrtps fastcdr)
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/cpp>
        ${ASIO_INCLUDE_DIR}
    )

###############################################################################
# Config
###############################################################################

# Install path
set(BIN_INSTALL_DIR bin/ CACHE PATH "Installation directory for binaries")
set(INCLUDE_INSTALL_DIR include/ CACHE PATH "Installation directory for C++ headers")
set(LIB_INSTALL_DIR lib/ CACHE PATH "Installation directory for libraries")
set(DATA_INSTALL_DIR share/ CACHE PATH "Installation directory for data")
set(DOC_INSTALL_DIR ${DOC_DIR} CACHE PATH "Installation directory for documentation")
if(WIN32)
    set(LICENSE_INSTALL_DIR . CACHE PATH "Installation directory for licenses")
else()
    set(LICENSE_INSTALL_DIR ${DATA_INSTALL_DIR}/${PROJECT_NAME} CACHE PATH "Installation directory for licenses")
endif()

###############################################################################
# Compile options
###############################################################################

# Set std
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    )

# Definition
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        -DBOOST_ASIO_STANDALONE
        -DASIO_STANDALONE
        -D$<$<BOOL:${VERBOSE}>:VERBOSE_OUTPUT>
    )

# Warnings
if(MSVC OR MSVC_IDE)
    target_compile_options(${PROJECT_NAME}
        PRIVATE
            /W4
            /wd4700
            /wd4996
            /wd4820
            /wd4255
            /wd4668
        )
else()
    target_compile_options(${PROJECT_NAME}
        PRIVATE
            -pedantic
            -fstrict-aliasing
            -Wall
            -Wextra
            -Wcast-align
            -Wshadow
        )
endif()

###############################################################################
# Testing
###############################################################################
if(EPROSIMA_BUILD_TESTS)

    # Create library for testing purposes
    add_library(micrortps-agent ${SRCS})
    target_link_libraries(micrortps-agent PUBLIC transport fastrtps fastcdr)
    target_include_directories(micrortps-agent
        PUBLIC
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
            $<INSTALL_INTERFACE:include>
            ${ASIO_INCLUDE_DIR}
        PRIVATE
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/cpp>
        )

    set_target_properties(micrortps-agent PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        )

    target_compile_definitions(micrortps-agent
        PUBLIC
            -DBOOST_ASIO_STANDALONE
            -DASIO_STANDALONE
        )

    # Enable tests
    enable_testing()
    add_subdirectory(test)
endif()

###############################################################################
# Packaging
###############################################################################
file(COPY ${PROJECT_SOURCE_DIR}/DEFAULT_FASTRTPS_PROFILES.xml
    DESTINATION ${PROJECT_BINARY_DIR}
    )

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    RUNTIME DESTINATION ${BIN_INSTALL_DIR}
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
    COMPONENT libraries
    )

export(TARGETS ${PROJECT_NAME} FILE ${PROJECT_BINARY_DIR}/cmake/config/${PROJECT_NAME}Targets.cmake)

install(EXPORT ${PROJECT_NAME}Targets
    DESTINATION ${LIB_INSTALL_DIR}/${PROJECT_NAME}/cmake
    COMPONENT cmake
    )

install(FILES ${PROJECT_SOURCE_DIR}/DEFAULT_FASTRTPS_PROFILES.xml
    DESTINATION ${BIN_INSTALL_DIR}
    )
