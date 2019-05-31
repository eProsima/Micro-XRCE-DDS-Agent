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

include(ExternalProject)

unset(_deps)


# Micro XRCE-DDS Client.
unset(microxrcedds_client_DIR CACHE)
find_package(microxrcedds_client "1.0.1" EXACT QUIET)
if(NOT microxrcedds_client_FOUND)
    ExternalProject_Add(uclient
        DOWNLOAD_COMMAND
            cd ${PROJECT_SOURCE_DIR} && git submodule update --init thirdparty/uxrclient/
        PREFIX
            ${PROJECT_BINARY_DIR}/uclient
        SOURCE_DIR
            ${PROJECT_SOURCE_DIR}/thirdparty/uxrclient
        INSTALL_DIR
            ${PROJECT_BINARY_DIR}/temp_install
        CMAKE_ARGS
            -DSUPERBUILD:BOOL=ON
            -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
            -DBUILD_SHARED_LIBS:BOOL=ON
        )
    list(APPEND _deps uclient)
endif()

# Fast CDR.
unset(fastcdr_DIR CACHE)
find_package(fastcdr "1.0.8" EXACT QUIET)
if(NOT microxrcedds_client_FOUND)
    ExternalProject_Add(fastcdr
        DOWNLOAD_COMMAND
            cd ${PROJECT_SOURCE_DIR} && git submodule update --init thirdparty/fastcdr/
        PREFIX
            ${PROJECT_BINARY_DIR}/fastcdr
        SOURCE_DIR
            ${PROJECT_SOURCE_DIR}/thirdparty/fastcdr
        INSTALL_DIR
            ${PROJECT_BINARY_DIR}/temp_install
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
            -DBUILD_SHARED_LIBS:BOOL=ON
        )
    list(APPEND _deps fastcdr)
endif()

# Fast RTPS.
unset(fastrtps_DIR CACHE)
find_package(fastrtps "1.7.2" EXACT QUIET)
if(NOT microxrcedds_client_FOUND)
    ExternalProject_Add(fastrtps
        DOWNLOAD_COMMAND
            cd ${PROJECT_SOURCE_DIR} && git submodule update --init thirdparty/fastrtps/
        PREFIX
            ${PROJECT_BINARY_DIR}/fastrtps
        SOURCE_DIR
            ${PROJECT_SOURCE_DIR}/thirdparty/fastrtps
        INSTALL_DIR
            ${PROJECT_BINARY_DIR}/temp_install
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
            -DBUILD_SHARED_LIBS:BOOL=ON
            -DCMAKE_PREFIX_PATH:PATH="${CMAKE_PREFIX_PATH};${PROJECT_BINARY_DIR}/temp_install"
            -DTHIRDPARTY:BOOL=ON
        DEPENDS
            fastcdr
        )
    list(APPEND _deps fastrtps)
endif()

# CLI11.
unset(CLI11_DIR CACHE)
find_package(CLI11 "1.7.1" EXACT QUIET)
if(NOT microxrcedds_client_FOUND)
    ExternalProject_Add(cli11
        DOWNLOAD_COMMAND
            cd ${PROJECT_SOURCE_DIR} && git submodule update --init thirdparty/CLI11/
        PREFIX
            ${PROJECT_BINARY_DIR}/CLI11
        SOURCE_DIR
            ${PROJECT_SOURCE_DIR}/thirdparty/CLI11
        INSTALL_DIR
            ${PROJECT_BINARY_DIR}/temp_install
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
            -DBUILD_SHARED_LIBS:BOOL=ON
            -DCMAKE_PREFIX_PATH:PATH="${CMAKE_PREFIX_PATH};${CMAKE_INSTALL_PREFIX}"
            -DCLI11_TESTING:BOOL=OFF
            -DCLI11_EXAMPLES:BOOL=OFF
        )
    list(APPEND _deps cli11)
endif()

# spdlog.
unset(spdlog_DIR CACHE)
find_package(spdlog "1.3.1" EXACT QUIET)
if(NOT microxrcedds_client_FOUND)
    ExternalProject_Add(spdlog
        DOWNLOAD_COMMAND
            cd ${PROJECT_SOURCE_DIR} && git submodule update --init thirdparty/spdlog/
        PREFIX
            ${PROJECT_BINARY_DIR}/spdlog
        SOURCE_DIR
            ${PROJECT_SOURCE_DIR}/thirdparty/spdlog
        INSTALL_DIR
            ${PROJECT_BINARY_DIR}/temp_install
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
            -DBUILD_SHARED_LIBS:BOOL=ON
            -DCMAKE_PREFIX_PATH:PATH="${CMAKE_PREFIX_PATH};${CMAKE_INSTALL_PREFIX}"
            -DSPDLOG_BUILD_EXAMPLES:BOOL=OFF
            -DSPDLOG_BUILD_TESTS:BOOL=OFF
            -DSPDLOG_INSTALL:BOOL=ON
        )
    list(APPEND _deps spdlog)
endif()

# Main project.
ExternalProject_Add(uagent
    SOURCE_DIR
        ${PROJECT_SOURCE_DIR}
    BINARY_DIR
        ${CMAKE_CURRENT_BINARY_DIR}
    CMAKE_ARGS
        -DUAGENT_SUPERBUILD=OFF
    INSTALL_COMMAND
        ""
    DEPENDS
        ${_deps}
    )
