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

###############################################################################
# CMake build rules for Micro XRCE-DDS Agent
###############################################################################
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

option(UAGENT_SUPERBUILD "Active super build" ON)

if(UAGENT_SUPERBUILD)
    project(uagent_superbuild NONE)
    include(${PROJECT_SOURCE_DIR}/cmake/SuperBuild.cmake)
    return()
else()
    project(microxrcedds_agent VERSION "1.0.1" LANGUAGES C CXX)
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
# Build options
###############################################################################
option(UAGENT_BUILD_TESTS "Activate the building tests" OFF)
option(EPROSIMA_INSTALLER "Activate the creation of a build to create Windows installer" OFF)

if(EPROSIMA_INSTALLER)
#    set(THIRDPARTY ON)
endif()

option(UAGENT_FAST_PROFILE "Build FastMiddleware profile." ON)
option(UAGENT_CED_PROFILE "Build CedMiddleware profile." ON)
option(UAGENT_DISCOVERY_PROFILE "Build Discovery profile." ON)
option(UAGENT_P2P_PROFILE "Build P2P discovery profile." ON)
option(UAGENT_LOGGER_PROFILE "Build logger profile." ON)

if(NOT UAGENT_CED_PROFILE)
    set(UAGENT_P2P_PROFILE OFF)
endif()

###############################################################################
# Config
###############################################################################
# Install path
include(GNUInstallDirs)
set(BIN_INSTALL_DIR     ${CMAKE_INSTALL_BINDIR}     CACHE PATH "Installation directory for binaries")
set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR} CACHE PATH "Installation directory for C headers")
set(LIB_INSTALL_DIR     ${CMAKE_INSTALL_LIBDIR}     CACHE PATH "Installation directory for libraries")
set(DATA_INSTALL_DIR    ${CMAKE_INSTALL_DATADIR}    CACHE PATH "Installation directory for data")
if(WIN32)
    set(LICENSE_INSTALL_DIR . CACHE PATH "Installation directory for licenses")
else()
    set(LICENSE_INSTALL_DIR ${DATA_INSTALL_DIR}/${PROJECT_NAME} CACHE PATH "Installation directory for licenses")
endif()

# Configuration options.
set(UAGENT_CONFIG_RELIABLE_STREAM_DEPTH        16  CACHE STRING "Reliable streams depth.")
set(UAGENT_CONFIG_BEST_EFFORT_STREAM_DEPTH     16  CACHE STRING "Best-effort streams depth.")
set(UAGENT_CONFIG_HEARTBEAT_PERIOD             200 CACHE STRING "Heartbeat period in milliseconds.")
set(UAGENT_CONFIG_TCP_MAX_CONNECTIONS          100 CACHE STRING "Maximum TCP connection allowed.")
set(UAGENT_CONFIG_TCP_MAX_BACKLOG_CONNECTIONS  100 CACHE STRING "Maximum TCP backlog connection allowed.")

# Create config.hpp
configure_file(${PROJECT_SOURCE_DIR}/include/uxr/agent/config.hpp.in
    ${PROJECT_BINARY_DIR}/include/uxr/agent/config.hpp
    )

###############################################################################
# Check MSVC architecture
###############################################################################
include(${PROJECT_SOURCE_DIR}/cmake/common/check_configuration.cmake)
if(MSVC OR MSVC_IDE)
    check_msvc_arch()
endif()

###############################################################################
# Load external dependencies.
###############################################################################
find_package(fastcdr "1.0.8" EXACT REQUIRED HINTS ${PROJECT_BINARY_DIR}/temp_install)
find_package(CLI11 "1.7.1" EXACT REQUIRED HINTS ${PROJECT_BINARY_DIR}/temp_install)
if(UAGENT_P2P_PROFILE)
    find_package(microxrcedds_client "1.0.1" EXACT REQUIRED HINTS ${PROJECT_BINARY_DIR}/temp_install)
    find_package(microcdr REQUIRED HINTS ${PROJECT_BINARY_DIR}/temp_install)
endif()
if(UAGENT_FAST_PROFILE)
    find_package(fastrtps "1.7.2" EXACT REQUIRED HINTS ${PROJECT_BINARY_DIR}/temp_install)
endif()
if(UAGENT_LOGGER_PROFILE)
    find_package(spdlog "1.3.1" EXACT REQUIRED HINTS ${PROJECT_BINARY_DIR}/temp_install)
endif()

###############################################################################
# Load external CMake Modules.
###############################################################################
if(CMAKE_VERSION VERSION_LESS 3.11.4)
    set(SANITIZER_PACKAGE "sanitizers-cmake")
    if(NOT EXISTS "${PROJECT_SOURCE_DIR}/thirdparty/${SANITIZER_PACKAGE}/CMakeLists.txt")
        message(STATUS "${SANITIZER_PACKAGE} thirdparty is being updated...")
        execute_process(
            COMMAND git submodule update --recursive --init "thirdparty/${SANITIZER_PACKAGE}"
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            RESULT_VARIABLE EXECUTE_RESULT
        )
        if(NOT EXECUTE_RESULT EQUAL 0)
            message(WARNING "Cannot configure Git submodule ${SANITIZER_PACKAGE}.")
        endif()
    endif()

    set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/thirdparty/${SANITIZER_PACKAGE}/cmake" ${CMAKE_MODULE_PATH})
    unset(SANITIZER_PACKAGE)
else()
    set(SANITIZER_PACKAGE "sanitizers-cmake")
    include(FetchContent)
    FetchContent_Declare(sanitizers-cmake
        GIT_REPOSITORY https://github.com/arsenm/sanitizers-cmake.git
        GIT_TAG master
    )
    FetchContent_GetProperties(sanitizers-cmake)
    if(NOT sanitizers-cmake_POPULATED)
        FetchContent_Populate(sanitizers-cmake)
        LIST(APPEND CMAKE_MODULE_PATH "${sanitizers-cmake_SOURCE_DIR}/cmake")
    endif()
endif()
find_package(Sanitizers REQUIRED)
if(SANITIZE_ADDRESS)
    message(STATUS "Preloading AddressSanitizer library could be done using \"${ASan_WRAPPER}\" to run your program.")
endif()

###############################################################################
# Sources
###############################################################################
# Check platform.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(TRANSPORT_SRCS
        src/cpp/transport/udp/UDPServerLinux.cpp
        src/cpp/transport/tcp/TCPServerLinux.cpp
        src/cpp/transport/serial/SerialServerLinux.cpp
        $<$<BOOL:${UAGENT_DISCOVERY_PROFILE}>:src/cpp/transport/discovery/DiscoveryServerLinux.cpp>
        $<$<BOOL:${UAGENT_P2P_PROFILE}>:src/cpp/transport/p2p/AgentDiscovererLinux.cpp>
        )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(TRANSPORT_SRCS
        src/cpp/transport/udp/UDPServerWindows.cpp
        src/cpp/transport/tcp/TCPServerWindows.cpp
        $<$<BOOL:${UAGENT_DISCOVERY_PROFILE}>:src/cpp/transport/discovery/DiscoveryServerWindows.cpp>
        )
endif()

# Set source files
set(SRCS
    src/cpp/Agent.cpp
    src/cpp/Root.cpp
    src/cpp/processor/Processor.cpp
    src/cpp/client/ProxyClient.cpp
    src/cpp/participant/Participant.cpp
    src/cpp/topic/Topic.cpp
    src/cpp/publisher/Publisher.cpp
    src/cpp/subscriber/Subscriber.cpp
    src/cpp/datawriter/DataWriter.cpp
    src/cpp/datareader/DataReader.cpp
    src/cpp/object/XRCEObject.cpp
    src/cpp/types/XRCETypes.cpp
    src/cpp/types/MessageHeader.cpp
    src/cpp/types/SubMessageHeader.cpp
    src/cpp/message/InputMessage.cpp
    src/cpp/message/OutputMessage.cpp
    src/cpp/transport/Server.cpp
    src/cpp/transport/udp/UDPServerBase.cpp
    src/cpp/transport/tcp/TCPServerBase.cpp
    src/cpp/transport/serial/SerialServerBase.cpp
    src/cpp/transport/serial/serial_protocol.c
    ${TRANSPORT_SRCS}
    $<$<BOOL:${UAGENT_DISCOVERY_PROFILE}>:src/cpp/transport/discovery/DiscoveryServer.cpp>
    $<$<BOOL:${UAGENT_FAST_PROFILE}>:src/cpp/types/TopicPubSubType.cpp>
    $<$<BOOL:${UAGENT_FAST_PROFILE}>:src/cpp/xmlobjects/xmlobjects.cpp>
    $<$<BOOL:${UAGENT_FAST_PROFILE}>:src/cpp/middleware/fast/FastEntities.cpp>
    $<$<BOOL:${UAGENT_FAST_PROFILE}>:src/cpp/middleware/fast/FastMiddleware.cpp>
    $<$<BOOL:${UAGENT_CED_PROFILE}>:src/cpp/middleware/ced/CedEntities.cpp>
    $<$<BOOL:${UAGENT_CED_PROFILE}>:src/cpp/middleware/ced/CedMiddleware.cpp>
    $<$<BOOL:${UAGENT_P2P_PROFILE}>:src/cpp/transport/p2p/AgentDiscoverer.cpp>
    $<$<BOOL:${UAGENT_P2P_PROFILE}>:src/cpp/p2p/InternalClientManager.cpp>
    $<$<BOOL:${UAGENT_P2P_PROFILE}>:src/cpp/p2p/InternalClient.cpp>
    )

###############################################################################
# Targets
###############################################################################
# Library
add_library(${PROJECT_NAME} ${SRCS})
add_sanitizers(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION
        ${PROJECT_VERSION}
    SOVERSION
        ${PROJECT_VERSION_MAJOR}
    CXX_STANDARD
        11
    CXX_STANDARD_REQUIRED
        YES
    )

get_target_property(UAGENT_TARGET_TYPE ${PROJECT_NAME} TYPE)
if((CMAKE_SYSTEM_NAME STREQUAL "Windows") AND (ACLIENT_TARGET_TYPE STREQUAL "SHARED_LIBRARY"))
    set_target_properties(${PROJECT_NAME} PROPERTIES
        RELEASE_POSTFIX
            -${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        RELWITHDEBINFO_POSTFIX
            -${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        DEBUG_POSTFIX
            d-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        )
    target_compile_definitions(${PROJECT_NAME}
        PUBLIC
            ${PROJECT_NAME}_SHARED
        )
endif()

target_compile_options(${PROJECT_NAME}
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-pedantic>
        $<$<CXX_COMPILER_ID:GNU>:-fstrict-aliasing>
        $<$<CXX_COMPILER_ID:GNU>:-Wall>
        $<$<CXX_COMPILER_ID:GNU>:-Wextra>
        $<$<CXX_COMPILER_ID:GNU>:-Wcast-align>
        $<$<CXX_COMPILER_ID:GNU>:-Wshadow>
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<CXX_COMPILER_ID:MSVC>:/wd4700>
        $<$<CXX_COMPILER_ID:MSVC>:/wd4996>
        $<$<CXX_COMPILER_ID:MSVC>:/wd4820>
        $<$<CXX_COMPILER_ID:MSVC>:/wd4255>
        $<$<CXX_COMPILER_ID:MSVC>:/wd4668>
    )

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        fastcdr
    PRIVATE
        $<$<BOOL:${UAGENT_FAST_PROFILE}>:fastrtps>
        $<$<BOOL:${UAGENT_LOGGER_PROFILE}>:spdlog::spdlog>
        $<$<BOOL:${UAGENT_P2P_PROFILE}>:microxrcedds_client>
        $<$<BOOL:${UAGENT_P2P_PROFILE}>:microcdr>
        $<$<PLATFORM_ID:Linux>:pthread>
    )

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/cpp>
    )

# Executable
add_executable(MicroXRCEAgent microxrce_agent.cpp)
target_link_libraries(MicroXRCEAgent
    PRIVATE
        ${PROJECT_NAME}
        $<$<BOOL:${UAGENT_LOGGER_PROFILE}>:spdlog::spdlog>
        CLI11::CLI11
    )

set_target_properties(MicroXRCEAgent PROPERTIES
    CXX_STANDARD
        11
    CXX_STANDARD_REQUIRED
        YES
    )

# XML default profile used to launch exec in the building folder
file(COPY ${PROJECT_SOURCE_DIR}/agent.refs
    DESTINATION ${PROJECT_BINARY_DIR}
    )

###############################################################################
# Testing
###############################################################################
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/modules)

if(UAGENT_BUILD_TESTS)
    include(${PROJECT_SOURCE_DIR}/cmake/common/gtest.cmake)
    find_package(GTest REQUIRED)
    find_package(GMock REQUIRED)
    find_package(Threads REQUIRED)
    enable_testing()
    include(CTest)
    if(GTEST_FOUND)
        if(UAGENT_FAST_PROFILE)
            add_subdirectory(test/unittest)
            add_subdirectory(test/unittest/agent)
            add_subdirectory(test/blackbox/tree)
        endif()
        if(UAGENT_CED_PROFILE)
            add_subdirectory(test/unittest/middleware/ced)
        endif()
        add_subdirectory(test/unittest/utils)
        add_subdirectory(test/unittest/types)
        add_subdirectory(test/unittest/transport/serial)
        add_subdirectory(test/unittest/client/session/stream)
    endif()
    add_subdirectory(test/integration/cross_serialization)
endif()

###############################################################################
# Packaging
###############################################################################

# Install agent lib
install(
    TARGETS
        ${PROJECT_NAME}
    EXPORT
        ${PROJECT_NAME}Targets
    RUNTIME DESTINATION
        ${BIN_INSTALL_DIR}
    LIBRARY DESTINATION
        ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION
        ${LIB_INSTALL_DIR}
    COMPONENT
        libraries
    )

# Install includes
install(
    DIRECTORY
        ${PROJECT_SOURCE_DIR}/include/uxr/agent/
    DESTINATION
        ${INCLUDE_INSTALL_DIR}/uxr/agent
    FILES_MATCHING
        PATTERN "*.hpp"
        PATTERN "*.h"
    )

# Export library
install(
    EXPORT
        ${PROJECT_NAME}Targets
    DESTINATION
        ${DATA_INSTALL_DIR}/${PROJECT_NAME}/cmake
    )

# Install agent exec
install(
    TARGETS
        MicroXRCEAgent
    EXPORT
        MicroXRCEAgentTargets
    RUNTIME
        DESTINATION ${BIN_INSTALL_DIR}
    LIBRARY
        DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE
        DESTINATION ${LIB_INSTALL_DIR}
    COMPONENT
        executables
    )

# Install config.h
install(
    FILES
        ${PROJECT_BINARY_DIR}/include/uxr/agent/config.hpp
    DESTINATION
        ${INCLUDE_INSTALL_DIR}/uxr/agent
    )

# Install licenses
install(
    FILES
        ${PROJECT_SOURCE_DIR}/LICENSE
    DESTINATION
        ${LICENSE_INSTALL_DIR}
    COMPONENT
        licenses
    )

# Package configuration
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/packaging/Config.cmake.in
        ${PROJECT_BINARY_DIR}/cmake/config/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION
        ${DATA_INSTALL_DIR}/${PROJECT_NAME}/cmake
    PATH_VARS
        BIN_INSTALL_DIR
        INCLUDE_INSTALL_DIR
        LIB_INSTALL_DIR
        DATA_INSTALL_DIR
    )

write_basic_package_version_file(
    ${PROJECT_BINARY_DIR}/cmake/config/${PROJECT_NAME}ConfigVersion.cmake
    VERSION
        ${PROJECT_VERSION}
    COMPATIBILITY
        SameMajorVersion
    )

install(
    FILES
        ${PROJECT_BINARY_DIR}/cmake/config/${PROJECT_NAME}Config.cmake
    DESTINATION
        ${DATA_INSTALL_DIR}/${PROJECT_NAME}/cmake
    )

# Install default profile XML.
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    install(
        FILES
            ${PROJECT_SOURCE_DIR}/agent.refs
        DESTINATION
            ${BIN_INSTALL_DIR}
        )
endif()

# Install dependencies
if(EXISTS ${CMAKE_BINARY_DIR}/temp_install/)
    install(
        DIRECTORY
            ${CMAKE_BINARY_DIR}/temp_install/
        DESTINATION
            ${CMAKE_INSTALL_PREFIX}
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
