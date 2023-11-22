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
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

###############################################################################
# Build options
###############################################################################
option(UAGENT_SUPERBUILD "Enable superbuild compilation." ON)
option(UAGENT_BUILD_TESTS "Build tests." OFF)
option(UAGENT_INSTALLER "Build Windows installer." OFF)
option(UAGENT_ISOLATED_INSTALL "Install the project and dependencies into separated folders with version control." OFF)
option(UAGENT_USE_INTERNAL_GTEST "Enable internal GTest libraries." OFF)
option(BUILD_SHARED_LIBS "Control shared/static building." ON)

option(UAGENT_USE_SYSTEM_FASTDDS "Force find and use system installed Fast-DDS." OFF)
option(UAGENT_USE_SYSTEM_FASTCDR "Force find and use system installed Fast-CDR." OFF)
option(UAGENT_USE_SYSTEM_LOGGER "Force find and use system installed spdlog." OFF)
option(UAGENT_FAST_PROFILE "Build FastMiddleware profile." ON)
option(UAGENT_CED_PROFILE "Build CedMiddleware profile." ON)
option(UAGENT_DISCOVERY_PROFILE "Build Discovery profile." ON)
option(UAGENT_P2P_PROFILE "Build P2P discovery profile." ON)
option(UAGENT_SOCKETCAN_PROFILE "Build Agent CAN FD transport." ON)
option(UAGENT_LOGGER_PROFILE "Build logger profile." ON)
option(UAGENT_SECURITY_PROFILE "Build security profile." OFF)
option(UAGENT_BUILD_EXECUTABLE "Build Micro XRCE-DDS Agent provided executable." ON)
option(UAGENT_BUILD_USAGE_EXAMPLES "Build Micro XRCE-DDS Agent built-in usage examples" OFF)

set(UAGENT_P2P_CLIENT_VERSION 2.4.2 CACHE STRING "Sets Micro XRCE-DDS client version for P2P")
set(UAGENT_P2P_CLIENT_TAG v2.4.2 CACHE STRING "Sets Micro XRCE-DDS client tag for P2P")

option(UAGENT_BUILD_CI_TESTS "Build CI test cases.")
if(UAGENT_BUILD_CI_TESTS)
    set(UAGENT_BUILD_TESTS ON)
endif()

if(NOT UAGENT_CED_PROFILE)
    set(UAGENT_P2P_PROFILE OFF)
endif()

if((CMAKE_SYSTEM_NAME STREQUAL "") AND (NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux"))
    set(UAGENT_P2P_PROFILE OFF)
endif()

if((CMAKE_SYSTEM_NAME STREQUAL "Darwin") OR (CMAKE_SYSTEM_NAME STREQUAL "Windows"))
    set(UAGENT_SOCKETCAN_PROFILE OFF)
endif()

set(UAGENT_CONFIG_RELIABLE_STREAM_DEPTH        16       CACHE STRING "Reliable streams depth.")
set(UAGENT_CONFIG_BEST_EFFORT_STREAM_DEPTH     16       CACHE STRING "Best-effort streams depth.")
set(UAGENT_CONFIG_HEARTBEAT_PERIOD             200      CACHE STRING "Heartbeat period in milliseconds.")
set(UAGENT_CONFIG_TCP_MAX_CONNECTIONS          100      CACHE STRING "Maximum TCP connection allowed.")
set(UAGENT_CONFIG_TCP_MAX_BACKLOG_CONNECTIONS  100      CACHE STRING "Maximum TCP backlog connection allowed.")
set(UAGENT_CONFIG_SERVER_QUEUE_MAX_SIZE        32000    CACHE STRING "Maximum server's queues size.")
set(UAGENT_CONFIG_CLIENT_DEAD_TIME             30000    CACHE STRING "Client dead time in milliseconds.")
set(UAGENT_SERVER_BUFFER_SIZE                  65535    CACHE STRING "Server buffer size.")

# Off-standard features and tweaks
option(UAGENT_TWEAK_XRCE_WRITE_LIMIT "This feature uses a tweak to allow XRCE WRITE DATA submessages greater than 64 kB." ON)

###############################################################################
# Dependencies
###############################################################################
set(_deps "")

if(UAGENT_USE_SYSTEM_FASTCDR)
    set(_fastcdr_version 1)
else()
    set(_fastcdr_version 1.1.1)
    set(_fastcdr_tag v1.1.1)
endif()
list(APPEND _deps "fastcdr\;${_fastcdr_version}")

if(UAGENT_P2P_PROFILE)
    set(_microxrcedds_client_version ${UAGENT_P2P_CLIENT_VERSION})
    set(_microxrcedds_client_tag ${UAGENT_P2P_CLIENT_TAG})
    list(APPEND _deps "microxrcedds_client\;${_microxrcedds_client_version}")
endif()

if(UAGENT_FAST_PROFILE)
    if(UAGENT_USE_SYSTEM_FASTDDS)
        set(_fastdds_version 2)
    else()
        set(_fastdds_version 2.12)
        set(_fastdds_tag 2.12.x)
        set(_foonathan_memory_tag v0.7-3) # This tag should be updated every time it gets updated in foonathan_memory_vendor eProsima's package
    endif()
    list(APPEND _deps "fastrtps\;${_fastdds_version}")
endif()

if(UAGENT_LOGGER_PROFILE)
    if(UAGENT_USE_SYSTEM_LOGGER)
        set(_spdlog_version 1)
    else()
        set(_spdlog_version 1.9.2)
        set(_spdlog_tag v1.9.2)
    endif()
    list(APPEND _deps "spdlog\;${_spdlog_version}")
endif()

###############################################################################
# Project
###############################################################################
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
if(NOT UAGENT_SUPERBUILD)
    project(microxrcedds_agent VERSION "2.4.2" LANGUAGES C CXX)
else()
    project(uagent_superbuild NONE)
    include(${PROJECT_SOURCE_DIR}/cmake/SuperBuild.cmake)
    return()
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
# Check MSVC architecture
###############################################################################
include(${PROJECT_SOURCE_DIR}/cmake/common/check_configuration.cmake)
if(MSVC OR MSVC_IDE)
    check_msvc_arch()
endif()

###############################################################################
# Load external dependencies.
###############################################################################
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${PROJECT_BINARY_DIR}/temp_install)

foreach(d ${_deps})
    list(GET d 0 _name)
    list(GET d 1 _version)
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${PROJECT_BINARY_DIR}/temp_install/${_name}-${_version})
    find_package(${_name} ${_version} REQUIRED)
endforeach()

###############################################################################
# Sources
###############################################################################
# Check platform.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Android" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(TRANSPORT_SRCS
        src/cpp/transport/udp/UDPv4AgentLinux.cpp
        src/cpp/transport/udp/UDPv6AgentLinux.cpp
        src/cpp/transport/tcp/TCPv4AgentLinux.cpp
        src/cpp/transport/tcp/TCPv6AgentLinux.cpp
        src/cpp/transport/serial/SerialAgentLinux.cpp
        src/cpp/transport/serial/TermiosAgentLinux.cpp
        src/cpp/transport/serial/MultiSerialAgentLinux.cpp
        src/cpp/transport/serial/MultiTermiosAgentLinux.cpp
        src/cpp/transport/serial/PseudoTerminalAgentLinux.cpp
        $<$<BOOL:${UAGENT_SOCKETCAN_PROFILE}>:src/cpp/transport/can/CanAgentLinux.cpp>
        $<$<BOOL:${UAGENT_DISCOVERY_PROFILE}>:src/cpp/transport/discovery/DiscoveryServerLinux.cpp>
        $<$<BOOL:${UAGENT_P2P_PROFILE}>:src/cpp/transport/p2p/AgentDiscovererLinux.cpp>
        )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(TRANSPORT_SRCS
        src/cpp/transport/udp/UDPv4AgentWindows.cpp
        src/cpp/transport/udp/UDPv6AgentWindows.cpp
        src/cpp/transport/tcp/TCPv4AgentWindows.cpp
        src/cpp/transport/tcp/TCPv6AgentWindows.cpp
        $<$<BOOL:${UAGENT_DISCOVERY_PROFILE}>:src/cpp/transport/discovery/DiscoveryServerWindows.cpp>
        )
endif()

# Set source files
set(SRCS
    src/cpp/Agent.cpp
    src/cpp/AgentInstance.cpp
    src/cpp/Root.cpp
    src/cpp/processor/Processor.cpp
    src/cpp/client/ProxyClient.cpp
    src/cpp/participant/Participant.cpp
    src/cpp/topic/Topic.cpp
    src/cpp/publisher/Publisher.cpp
    src/cpp/subscriber/Subscriber.cpp
    src/cpp/datawriter/DataWriter.cpp
    src/cpp/datareader/DataReader.cpp
    src/cpp/requester/Requester.cpp
    src/cpp/replier/Replier.cpp
    src/cpp/object/XRCEObject.cpp
    src/cpp/types/XRCETypes.cpp
    src/cpp/types/MessageHeader.cpp
    src/cpp/types/SubMessageHeader.cpp
    src/cpp/message/InputMessage.cpp
    src/cpp/message/OutputMessage.cpp
    src/cpp/utils/ArgumentParser.cpp
    src/cpp/transport/Server.cpp
    src/cpp/transport/stream_framing/StreamFramingProtocol.cpp
    src/cpp/transport/custom/CustomAgent.cpp
    ${TRANSPORT_SRCS}
    $<$<BOOL:${UAGENT_DISCOVERY_PROFILE}>:src/cpp/transport/discovery/DiscoveryServer.cpp>
    $<$<BOOL:${UAGENT_FAST_PROFILE}>:src/cpp/types/TopicPubSubType.cpp>
    $<$<BOOL:${UAGENT_FAST_PROFILE}>:src/cpp/xmlobjects/xmlobjects.cpp>
    $<$<BOOL:${UAGENT_FAST_PROFILE}>:src/cpp/middleware/fast/FastEntities.cpp>
    $<$<BOOL:${UAGENT_FAST_PROFILE}>:src/cpp/middleware/fast/FastMiddleware.cpp>
    $<$<BOOL:${UAGENT_FAST_PROFILE}>:src/cpp/middleware/fastdds/FastDDSEntities.cpp>
    $<$<BOOL:${UAGENT_FAST_PROFILE}>:src/cpp/middleware/fastdds/FastDDSMiddleware.cpp>
    $<$<BOOL:${UAGENT_CED_PROFILE}>:src/cpp/middleware/ced/CedEntities.cpp>
    $<$<BOOL:${UAGENT_CED_PROFILE}>:src/cpp/middleware/ced/CedMiddleware.cpp>
    $<$<BOOL:${UAGENT_P2P_PROFILE}>:src/cpp/transport/p2p/AgentDiscoverer.cpp>
    $<$<BOOL:${UAGENT_P2P_PROFILE}>:src/cpp/p2p/InternalClientManager.cpp>
    $<$<BOOL:${UAGENT_P2P_PROFILE}>:src/cpp/p2p/InternalClient.cpp>
    )

###############################################################################
# Set install directories
###############################################################################
if(UAGENT_ISOLATED_INSTALL)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}-${PROJECT_VERSION}")
endif()

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

###############################################################################
# Targets
###############################################################################
# Library
add_library(${PROJECT_NAME} ${SRCS})

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION
        ${PROJECT_VERSION}
    SOVERSION
        ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    CXX_STANDARD
        11
    CXX_STANDARD_REQUIRED
        YES
    C_STANDARD
        99
    C_STANDARD_REQUIRED
        YES
    )

get_target_property(UAGENT_TARGET_TYPE ${PROJECT_NAME} TYPE)
if((CMAKE_SYSTEM_NAME STREQUAL "Windows") AND (UAGENT_TARGET_TYPE STREQUAL "SHARED_LIBRARY"))
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
        $<$<PLATFORM_ID:Windows>:ws2_32>
        $<$<PLATFORM_ID:Windows>:iphlpapi>
        $<$<BOOL:${UAGENT_LOGGER_PROFILE}>:spdlog::spdlog>
    PRIVATE
        $<$<BOOL:${UAGENT_FAST_PROFILE}>:fastrtps>
        $<$<BOOL:${UAGENT_P2P_PROFILE}>:microxrcedds_client>
        $<$<BOOL:${UAGENT_P2P_PROFILE}>:microcdr>
        $<$<PLATFORM_ID:Linux>:pthread>
    )

target_include_directories(${PROJECT_NAME} BEFORE
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>
        $<TARGET_PROPERTY:fastcdr,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/cpp>
        $<$<BOOL:${UAGENT_FAST_PROFILE}>:$<TARGET_PROPERTY:fastrtps,INTERFACE_INCLUDE_DIRECTORIES>>
        $<$<BOOL:${UAGENT_LOGGER_PROFILE}>:$<TARGET_PROPERTY:spdlog::spdlog,INTERFACE_INCLUDE_DIRECTORIES>>
        $<$<BOOL:${UAGENT_P2P_PROFILE}>:$<TARGET_PROPERTY:microxrcedds_client,INTERFACE_INCLUDE_DIRECTORIES>>
        $<$<BOOL:${UAGENT_P2P_PROFILE}>:$<TARGET_PROPERTY:microcdr,INTERFACE_INCLUDE_DIRECTORIES>>
    )

# Executable
if(UAGENT_BUILD_EXECUTABLE)
    add_executable(MicroXRCEAgent microxrce_agent.cpp)
    target_link_libraries(MicroXRCEAgent
        PRIVATE
            ${PROJECT_NAME}
            $<$<BOOL:$<PLATFORM_ID:Linux>>:rt>
            $<$<BOOL:$<PLATFORM_ID:Linux>>:dl>
        )

    set_target_properties(MicroXRCEAgent PROPERTIES
        CXX_STANDARD
            11
        CXX_STANDARD_REQUIRED
            YES
        )
endif()

# Examples
if(UAGENT_BUILD_USAGE_EXAMPLES)
    add_subdirectory(examples/custom_agent)
endif()

# XML default profile used to launch exec in the building folder
file(COPY ${PROJECT_SOURCE_DIR}/agent.refs
    DESTINATION ${PROJECT_BINARY_DIR}
    )

###############################################################################
# Testing
###############################################################################
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/modules)

if(UAGENT_BUILD_CI_TESTS OR UAGENT_BUILD_TESTS)
    include(CTest)
endif()

if(UAGENT_BUILD_CI_TESTS)
    add_subdirectory(test/case)
endif()

if(UAGENT_BUILD_TESTS)
    include(${PROJECT_SOURCE_DIR}/cmake/common/gtest.cmake)
    if(UAGENT_USE_INTERNAL_GTEST)
        unset(ENV{GTEST_ROOT})
        unset(ENV{GMOCK_ROOT})
    endif()
    find_package(GTest REQUIRED)
    find_package(GMock REQUIRED)
    find_package(Threads REQUIRED)

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
    add_subdirectory(test/unittest/client/session/stream)
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        add_subdirectory(test/unittest/transport/serial)
    endif()
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
if(UAGENT_BUILD_EXECUTABLE)
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
endif()

# Create config.hpp
configure_file(${PROJECT_SOURCE_DIR}/include/uxr/agent/config.hpp.in
    ${PROJECT_BINARY_DIR}/include/uxr/agent/config.hpp
    )

# Install config.hpp
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
        ${PROJECT_BINARY_DIR}/cmake/config/${PROJECT_NAME}ConfigVersion.cmake
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

# Install dependencies.
if(EXISTS ${CMAKE_BINARY_DIR}/temp_install/)
    file(GLOB _deps RELATIVE ${CMAKE_BINARY_DIR}/temp_install ${CMAKE_BINARY_DIR}/temp_install/*)
    foreach(_d ${_deps})
        install(
            DIRECTORY
                ${CMAKE_BINARY_DIR}/temp_install/${_d}$<IF:$<BOOL:${UAGENT_ISOLATED_INSTALL}>,,/>
            DESTINATION
                ${CMAKE_INSTALL_PREFIX}$<IF:$<BOOL:${UAGENT_ISOLATED_INSTALL}>,/../,>
            COMPONENT
                ${_d}
            USE_SOURCE_PERMISSIONS
        )
    endforeach()
endif()

# Generate Windows Installer.
if(WIN32 AND UAGENT_INSTALLER AND UAGENT_BUILD_EXECUTABLE)
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
        file(TO_NATIVE_PATH $ENV{PROGRAMFILES}/eProsima _install_root)
        set(CPACK_NSIS_INSTALL_ROOT ${_install_root})
    else()
        set(PF86_STR "PROGRAMFILES(x86)")
        set(CPACK_NSIS_INSTALL_ROOT "$ENV{${PF86_STR}}\\eProsima")
    endif()

    # Setup configure files for NSIS installer.
    configure_file(${PROJECT_SOURCE_DIR}/cmake/packaging/windows/NSISPackaging.cmake.in
        ${PROJECT_BINARY_DIR}/cmake/packaging/windows/NSISPackaging.cmake @ONLY
        )
    # Setup redistributables installation nsh.
    if(${MSVC_ARCH} STREQUAL "x64Win64VS2015" OR ${MSVC_ARCH} STREQUAL "x64Win64VS2017" OR ${MSVC_ARCH} STREQUAL "x64Win64VS2019")
        set(REDIST_REGISTER "SOFTWARE\\Classes\\Installer\\Dependencies\\VC,redist.x64,amd64,14.21,bundle")
        set(REDIST_VERSION "14.21.27702.2")
        set(REDIST_LINK "https://aka.ms/vs/16/release/VC_redist.x64.exe")
        set(REDIST_EXEC "VC_redist.x64.exe")
    elseif(${MSVC_ARCH} STREQUAL "i86Win32VS2015" OR ${MSVC_ARCH} STREQUAL "i86Win32VS2017" OR ${MSVC_ARCH} STREQUAL "i86Win32VS2019")
        set(REDIST_REGISTER "SOFTWARE\\Classes\\Installer\\Dependencies\\VC,redist.x86,x86,14.21,bundle")
        set(REDIST_VERSION "14.21.27702.2")
        set(REDIST_LINK "https://aka.ms/vs/16/release/VC_redist.x86.exe")
        set(REDIST_EXEC "VC_redist.x86.exe")
    endif()

    configure_file(${PROJECT_SOURCE_DIR}/cmake/packaging/windows/InstallRedistributables.nsh.in
        ${PROJECT_BINARY_DIR}/cmake/packaging/windows/InstallRedistributables.nsh @ONLY
        )

    # Setup CPack install script
    set(CPACK_INSTALL_SCRIPT "${PROJECT_BINARY_DIR}/cmake/packaging/windows/NSISPackaging.cmake")

    # Update CMAKE_MODULE_PATH to find NSIS.template.in
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/packaging/windows)

    include(CPack)
endif()
