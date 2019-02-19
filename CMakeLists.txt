# Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
# CMake build rules for Micro XRCE-DDS Agent
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
    project(microxrcedds_agent C CXX)
    set(PROJECT_VERSION_MAJOR 1)
    set(PROJECT_VERSION_MINOR 0)
    set(PROJECT_VERSION_PATCH 1)
    set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})
else()
    cmake_policy(SET CMP0048 NEW)
    project(microxrcedds_agent VERSION "1.0.1" LANGUAGES C CXX)
endif()

###############################################################################
# eProsima build options
###############################################################################
option(EPROSIMA_BUILD "Activate internal building" OFF)
option(EPROSIMA_BUILD_TESTS "Activate the building tests" OFF)
option(THIRDPARTY "Activate the build of thirdparties" OFF)
option(VERBOSE "Use verbose output" OFF)
option(EPROSIMA_INSTALLER "Activate the creation of a build to create Windows installer" OFF)

if(EPROSIMA_INSTALLER)
    set(THIRDPARTY ON)
endif()

if(EPROSIMA_BUILD)
    set(THIRDPARTY ON)
    set(EPROSIMA_BUILD_TESTS ON)
endif()

###############################################################################
# Check MSVC architecture
###############################################################################
include(${PROJECT_SOURCE_DIR}/cmake/common/check_configuration.cmake)
if(MSVC OR MSVC_IDE)
    check_msvc_arch()
endif()

###############################################################################
# Config
###############################################################################

# Install path
set(BIN_INSTALL_DIR bin/ CACHE PATH "Installation directory for binaries")
set(INCLUDE_INSTALL_DIR include/ CACHE PATH "Installation directory for C++ headers")
set(LIB_INSTALL_DIR lib/ CACHE PATH "Installation directory for libraries")
set(DATA_INSTALL_DIR share/ CACHE PATH "Installation directory for data")
if(WIN32 AND EPROSIMA_INSTALLER AND IS_TOP_LEVEL)
    set(LICENSE_INSTALL_DIR . CACHE PATH "Installation directory for licenses")
else()
    set(LICENSE_INSTALL_DIR ${DATA_INSTALL_DIR}/${PROJECT_NAME} CACHE PATH "Installation directory for licenses")
endif()

# Configuration options.
set(CONFIG_RELIABLE_STREAM_DEPTH 16 CACHE STRING "Reliable streams depth.")
set(CONFIG_BEST_EFFORT_STREAM_DEPTH 16 CACHE STRING "Best-effort streams depth.")
set(CONFIG_HEARTBEAT_PERIOD 200 CACHE STRING "Heartbeat period in milliseconds.")
set(CONFIG_TCP_MAX_CONNECTIONS 100 CACHE STRING "Maximum TCP connection allowed.")
set(CONFIG_TCP_MAX_BACKLOG_CONNECTIONS 100 CACHE STRING "Maximum TCP backlog connection allowed.")

# Create source files with the define
configure_file(${PROJECT_SOURCE_DIR}/include/uxr/agent/config.hpp.in
    ${PROJECT_BINARY_DIR}/include/uxr/agent/config.hpp
    )

###############################################################################
# Load external eProsima projects.
###############################################################################
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/modules)
include(${PROJECT_SOURCE_DIR}/cmake/common/eprosima_libraries.cmake)
eprosima_find_package(fastcdr REQUIRED)
eprosima_find_package(fastrtps REQUIRED)
eprosima_find_thirdparty(Asio asio)

###############################################################################
# Targets
###############################################################################

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(TRANSPORT_SRCS
        src/cpp/transport/udp/UDPServerLinux.cpp
        src/cpp/transport/tcp/TCPServerLinux.cpp
        src/cpp/transport/serial/SerialServerLinux.cpp
        src/cpp/transport/discovery/DiscoveryServerLinux.cpp
        )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(TRANSPORT_SRCS
        src/cpp/transport/udp/UDPServerWindows.cpp
        src/cpp/transport/tcp/TCPServerWindows.cpp
        src/cpp/transport/discovery/DiscoveryServerWindows.cpp
        )
endif()

# Set source files
set(SRCS
    src/cpp/Root.cpp
    src/cpp/processor/Processor.cpp
    src/cpp/client/ProxyClient.cpp
    src/cpp/participant/Participant.cpp
    src/cpp/topic/Topic.cpp
    src/cpp/publisher/Publisher.cpp
    src/cpp/subscriber/Subscriber.cpp
    src/cpp/datawriter/DataWriter.cpp
    src/cpp/datareader/DataReader.cpp
    src/cpp/datareader/TokenBucket.cpp
    src/cpp/object/XRCEObject.cpp
    src/cpp/types/XRCETypes.cpp
    src/cpp/types/MessageHeader.cpp
    src/cpp/types/SubMessageHeader.cpp
    $<$<BOOL:${VERBOSE}>:src/cpp/libdev/MessageOutput.cpp>
    src/cpp/transport/Server.cpp
    src/cpp/transport/udp/UDPServerBase.cpp
    src/cpp/transport/tcp/TCPServerBase.cpp
    src/cpp/transport/serial/SerialServerBase.cpp
    src/cpp/transport/serial/serial_protocol.c
    src/cpp/transport/discovery/DiscoveryServer.cpp
    ${TRANSPORT_SRCS}
    src/cpp/types/TopicPubSubType.cpp
    src/cpp/xmlobjects/xmlobjects.cpp
    src/cpp/middleware/fast/FastEntities.cpp
    src/cpp/middleware/fast/FastMiddleware.cpp
    src/cpp/middleware/ced/CedEntities.cpp
    src/cpp/middleware/ced/CedMiddleware.cpp
    )

# Executable
add_library(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} PUBLIC fastrtps fastcdr)
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/cpp>
        ${ASIO_INCLUDE_DIR}
    )
add_executable(MicroXRCEAgent microxrce_agent.cpp)
target_link_libraries(MicroXRCEAgent PRIVATE ${PROJECT_NAME})

# XML default profile used to launch exec in the building folder
file(COPY ${PROJECT_SOURCE_DIR}/agent.refs
    DESTINATION ${PROJECT_BINARY_DIR}
    )

###############################################################################
# Compile options
###############################################################################

# Set targets properties.
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    )
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        RELEASE_POSTFIX -${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        RELWITHDEBINFO_POSTFIX -${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        DEBUG_POSTFIX d-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        )
endif()
set_target_properties(MicroXRCEAgent PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    )

# Definition
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        BOOST_ASIO_STANDALONE
        ASIO_STANDALONE
        $<$<BOOL:${VERBOSE}>:VERBOSE_OUTPUT>
    )

get_target_property(TARGET_TYPE ${PROJECT_NAME} TYPE)
if((MSVC OR MSVC_IDE) AND (TARGET_TYPE STREQUAL "SHARED_LIBRARY"))
    target_compile_definitions(${PROJECT_NAME}
        PUBLIC
            -D${PROJECT_NAME}_SHARED
        )
endif()

# Warnings
if(MSVC OR MSVC_IDE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4 /wd4700 /wd4996 /wd4820 /wd4255 /wd4668")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /wd4700 /wd4996 /wd4820 /wd4255 /wd4668")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -fstrict-aliasing -Wall -Wextra -Wcast-align -Wshadow")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -fstrict-aliasing -Wall -Wextra -Wcast-align")
endif()

###############################################################################
# Testing
###############################################################################
if(EPROSIMA_BUILD_TESTS AND IS_TOP_LEVEL)
    include(${PROJECT_SOURCE_DIR}/cmake/common/gtest.cmake)
    check_gtest()
    check_gmock()
    enable_testing()
    include(CTest)
    if(GTEST_FOUND)
        add_subdirectory(test/unittest/root)
        add_subdirectory(test/unittest/util)
        add_subdirectory(test/unittest/xrce)
        add_subdirectory(test/unittest/transport/serial)
        # TODO (#5047): enable when XML is available
        #add_subdirectory(test/blackbox/tree)
    endif()
    add_subdirectory(test/integration/cross_serialization)
endif()

###############################################################################
# Packaging
###############################################################################

# Install agent lib
install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    RUNTIME DESTINATION ${BIN_INSTALL_DIR}
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
    COMPONENT libraries
    )

# Install includes
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/uxr/agent/
    DESTINATION ${INCLUDE_INSTALL_DIR}/uxr/agent
    FILES_MATCHING PATTERN "*.hpp"
                   PATTERN "*.h"
    )

# Export library
install(EXPORT ${PROJECT_NAME}Targets
    DESTINATION ${DATA_INSTALL_DIR}/${PROJECT_NAME}/cmake
    )

# Install agent exec
install(TARGETS MicroXRCEAgent
    EXPORT MicroXRCEAgentTargets
    RUNTIME DESTINATION ${BIN_INSTALL_DIR}
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
    COMPONENT executables
    )

# Install config.h
install(FILES ${PROJECT_BINARY_DIR}/include/uxr/agent/config.hpp
    DESTINATION ${INCLUDE_INSTALL_DIR}/uxr/agent
    )

# Install licenses
install(FILES ${PROJECT_SOURCE_DIR}/LICENSE
    DESTINATION ${LICENSE_INSTALL_DIR}
    COMPONENT licenses
    )

# Package configuration
include(CMakePackageConfigHelpers)
configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/packaging/Config.cmake.in
    ${PROJECT_BINARY_DIR}/cmake/config/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION ${DATA_INSTALL_DIR}/${PROJECT_NAME}/cmake
    PATH_VARS BIN_INSTALL_DIR INCLUDE_INSTALL_DIR LIB_INSTALL_DIR DATA_INSTALL_DIR
    )
install(FILES ${PROJECT_BINARY_DIR}/cmake/config/${PROJECT_NAME}Config.cmake
    DESTINATION ${DATA_INSTALL_DIR}/${PROJECT_NAME}/cmake
    )

# Install default profile XML.
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    install(FILES ${PROJECT_SOURCE_DIR}/agent.refs
        DESTINATION ${BIN_INSTALL_DIR}
        )
endif()

# Generate Windows Installer.
if(WIN32 AND EPROSIMA_INSTALLER)
    # Set product name.
    set(PRODUCT_NAME_SHORT "MicroXRCEAgent")
    set(PRODUCT_NAME_LONG "Micro XRCE-DDS Agent")

    set(CPACK_GENERATOR NSIS)

    # Setup generic options.
    set(CPACK_PACKAGE_NAME ${PRODUCT_NAME_SHORT})
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++ implementarion of XRCE Root")
    set(CPACK_PACKAGE_DESCRIPTION "eProsima ${PRODUCT_NAME_SHORT} is a executable which implement the XRCE Root functionality.")
    set(CPACK_PACKAGE_VENDOR "eProsima")
    set(CPACK_PACKAGE_CONTACT "eProsima Support <support@eprosima.com>")
    set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_MAJOR_VERSION})
    set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_MINOR_VERSION})
    set(CPACK_PACKAGE_VERSION_PATH ${PROJECT_MICRO_VERSION})
    set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
    set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
    set(CPACK_COMPONENTS_LICENSES_HIDDEN 1)

    # Components display.
    set(CPACK_COMPONENT_CMAKE_DISPLAY_NAME "CMake files")
    set(CPACK_COMPONENT_CMAKE_DESCRIPTION "CMake configuration files")
    set(CPACK_COMPONENT_HEADERS_DESCRIPTION "eProsima ${PRODUCT_NAME_SHORT} C++ Headers")
    set(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "C++ Headers")
    set(CPACK_COMPONENT_HEADERS_DESCRIPTION "eProsima ${PRODUCT_NAME_SHORT} C++ Headers")
    string(TOUPPER "${MSVC_ARCH}" MSVC_ARCH_UPPER)
    set(CPACK_COMPONENT_LIBRARIES_${MSVC_ARCH_UPPER}_DISPLAY_NAME "${MSCV_ARCH}")
    set(CPACK_COMPONENT_LIBRARIES_${MSVC_ARCH_UPPER}_DESCRIPTION "eProsima ${PRODUCT_NAME_SHORT} libraries form platform ${MSVC_ARCH}")
    set(CPACK_COMPONENT_LICENSES_DISPLAY_NAME "License")
    set(CPACK_COMPONENT_LICENSES_DESCRIPTION "eProsima ${PRODUCT_NAME_SHORT} License")

    # Setup NSIS installer.
    set(CPACK_NSIS_DISPLAY_NAME "${PRODUCT_NAME_LONG} ${PROJECT_VERSION}")
    set(CPACK_NSIS_PACKAGE_NAME "eProsima ${PRODUCT_NAME_LONG} ${PROJECT_VERSION}")
    set(CPACK_NSIS_URL_INFO_ABOUT "www.eprosima.com")
    set(CPACK_NSIS_CONTACT "support@eprosima.com")
    set(CPACK_NSIS_MUI_ICON "${PROJECT_SOURCE_DIR}/utils/images/icon/eprosima_icon.ico")
    set(CPACK_NSIS_MENU_LINKS 
        "bin/MicroXRCEAgent.exe" "${PRODUCT_NAME_SHORT}"
        "Uninstall.exe" "Uninstall ${PRODUCT_NAME_SHORT}"
        )
    if(CMAKE_CL_64)
	set(CPACK_NSIS_INSTALL_ROOT "$ENV{PROGRAMFILES}\\eProsima")
    else() 
        set(PF86_STR "PROGRAMFILES(x86)")
	set(CPACK_NSIS_INSTALL_ROOT "$ENV{${PF86_STR}}\\eProsima")
    endif()

    # Setup configure files for NSIS installer.
    configure_file(${PROJECT_SOURCE_DIR}/cmake/packaging/windows/NSISPackaging.cmake.in
        ${PROJECT_BINARY_DIR}/cmake/packaging/windows/NSISPackaging.cmake @ONLY
        )
    # Setup redistributables installation nsh.
    if(${MSVC_ARCH} STREQUAL "x64Win64VS2017")
        set(REDIST_REGISTER "SOFTWARE\\Classes\\Installer\\Dependencies\\,,amd64,14.0,bundle")
	set(REDIST_VERSION "14.15.26706.0")
	set(REDIST_LINK "https://download.visualstudio.microsoft.com/download/pr/20ef12bb-5283-41d7-90f7-eb3bb7355de7/8b58fd89f948b2430811db3da92299a6/vc_redist.x64.exe")
	set(REDIST_EXEC "vc_redist_x64.exe")
    elseif(${MSVC_ARCH} STREQUAL "i86Win32VS2017")
        set(REDIST_REGISTER "SOFTWARE\\Classes\\Installer\\Dependencies\\,,x86,14.0,bundle")
	set(REDIST_VERSION "14.15.26706.0")
	set(REDIST_LINK "https://download.visualstudio.microsoft.com/download/pr/749aa419-f9e4-4578-a417-a43786af205e/d59197078cc425377be301faba7dd87a/vc_redist.x86.exe")
	set(REDIST_EXEC "vc_redist_i86.exe")
    endif()
    configure_file(${PROJECT_SOURCE_DIR}/cmake/packaging/windows/InstallRedistributables.nsh.in
        ${PROJECT_BINARY_DIR}/cmake/packaging/windows/InstallRedistributables.nsh @ONLY
        )

    # Setup CPack install script
    set(CPACK_INSTALL_SCRIPT "${PROJECT_BINARY_DIR}/cmake/packaging/windows/NSISPackaging.cmake")

    # Update CMAKE_MODULE_PATH to find NSIS.template.in
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/packaging/windows")	

    include(CPack)
endif()
