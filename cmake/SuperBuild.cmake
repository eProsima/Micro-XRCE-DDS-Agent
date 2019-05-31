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
find_package(microxrcedds_client "1.0.1" EXACT QUIET)
if(NOT microxrcedds_client_FOUND)
    ExternalProject_Add(uclient
        DOWNLOAD_COMMAND
            git submodule update --init ${PROJECT_SOURCE_DIR}/thirdparty/uxrclient/
        PREFIX
            ${PROJECT_BINARY_DIR}/uclient
        SOURCE_DIR
            ${PROJECT_SOURCE_DIR}/thirdparty/fastcdr
        INSTALL_DIR
            ${PROJECT_BINARY_DIR}/temp_install
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
            -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
        )
    list(APPEND _deps uclient)
endif()

# Fast CDR.
find_package(fastcdr "1.0.8" EXACT QUIET)
if(NOT microxrcedds_client_FOUND)
    ExternalProject_Add(fastcdr
        DOWNLOAD_COMMAND
            git submodule update --init ${PROJECT_SOURCE_DIR}/thirdparty/fastcdr/
        PREFIX
            ${PROJECT_BINARY_DIR}/fastcdr
        SOURCE_DIR
            ${PROJECT_SOURCE_DIR}/thirdparty/fastcdr
        INSTALL_DIR
            ${PROJECT_BINARY_DIR}/temp_install
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
            -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
            -DTHIRDPARTY:BOOL=ON
        )
    list(APPEND _deps fastcdr)
endif()

# Fast RTPS.
find_package(fastrtps "1.7.2" EXACT QUIET)
if(NOT microxrcedds_client_FOUND)
    ExternalProject_Add(fastrtps
        DOWNLOAD_COMMAND
            git submodule update --init ${PROJECT_SOURCE_DIR}/thirdparty/fastrtps/
        PREFIX
            ${PROJECT_BINARY_DIR}/fastrtps
        SOURCE_DIR
            ${PROJECT_SOURCE_DIR}/thirdparty/fastrtps
        INSTALL_DIR
            ${PROJECT_BINARY_DIR}/temp_install
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
            -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
            -DCMAKE_PREFIX_PATH:PATH="${CMAKE_PREFIX_PATH};${CMAKE_INSTALL_PREFIX}"
            -DTHIRDPARTY:BOOL=ON
        )
    list(APPEND _deps fastrtps)
endif()

# CLI11.
#find_package(CLI11 "1.7.1" EXACT QUIET)
#if(NOT microxrcedds_client_FOUND)
#    ExternalProject_Add(fastrtps
#        DOWNLOAD_COMMAND
#            git submodule update --init ${PROJECT_SOURCE_DIR}/thirdparty/fastrtps/
#        PREFIX
#            ${PROJECT_BINARY_DIR}/fastrtps
#        SOURCE_DIR
#            ${PROJECT_SOURCE_DIR}/thirdparty/fastrtps
#        INSTALL_DIR
#            ${PROJECT_BINARY_DIR}/temp_install
#        CMAKE_ARGS
#            -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
#            -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
#            -DCMAKE_PREFIX_PATH:PATH="${CMAKE_PREFIX_PATH};${CMAKE_INSTALL_PREFIX}"
#            -DTHIRDPARTY:BOOL=ON
#        )
#    list(APPEND _deps fastrtps)
#endif()

# Main project.
ExternalProject_Add(uagent
    SOURCE_DIR
        ${PROJECT_SOURCE_DIR}
    BINARY_DIR
        ${CMAKE_CURRENT_BINARY_DIR}
    CMAKE_ARGS
        -DSUPERBUILD=OFF
        -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    INSTALL_COMMAND
        ""
    DEPENDS
        ${_deps}
    )
