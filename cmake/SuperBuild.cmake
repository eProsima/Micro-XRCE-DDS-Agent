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

if(UAGENT_P2P_PROFILE)
    # Micro XRCE-DDS Client.
    unset(microxrcedds_client_DIR CACHE)
    find_package(microxrcedds_client "1.1.0" EXACT QUIET)
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
                -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
                -DBUILD_SHARED_LIBS:BOOL=ON
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            )
        list(APPEND _deps uclient)
    endif()
endif()

# Fast CDR.
unset(fastcdr_DIR CACHE)
find_package(fastcdr "1.0.9" EXACT QUIET)
if(NOT fastcdr_FOUND)
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
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        )
    list(APPEND _deps fastcdr)
endif()

if(UAGENT_FAST_PROFILE)
    # Fast RTPS.
    unset(fastrtps_DIR CACHE)
    find_package(fastrtps "1.8.1" EXACT QUIET)
    if(NOT fastrtps_FOUND)
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
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                -DTHIRDPARTY:BOOL=ON
            DEPENDS
                fastcdr
            )
        list(APPEND _deps fastrtps)
    endif()
endif()

# CLI11.
unset(CLI11_DIR CACHE)
find_package(CLI11 "1.7.1" EXACT QUIET)
if(NOT CLI11_FOUND)
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
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCLI11_TESTING:BOOL=OFF
            -DCLI11_EXAMPLES:BOOL=OFF
        )
    list(APPEND _deps cli11)
endif()

if(UAGENT_LOGGER_PROFILE)
    # spdlog.
    unset(spdlog_DIR CACHE)
    find_package(spdlog "1.3.1" EXACT QUIET)
    if(NOT spdlog_FOUND)
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
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                -DSPDLOG_BUILD_EXAMPLES:BOOL=OFF
                -DSPDLOG_BUILD_TESTS:BOOL=OFF
                -DSPDLOG_INSTALL:BOOL=ON
            )
        list(APPEND _deps spdlog)
    endif()
endif()

if(UAGENT_BUILD_TESTS)
    unset(googletest_DIR CACHE)
    enable_language(CXX)
    find_package(GTest QUIET)
    find_package(GMock QUIET)
    if(NOT GTest_FOUND OR NOT GMock_FOUND OR UAGENT_USE_INTERNAL_GTEST)
        unset(GTEST_ROOT CACHE)
        unset(GMOCK_ROOT CACHE)
        ExternalProject_Add(googletest
            GIT_REPOSITORY
                https://github.com/google/googletest.git
            GIT_TAG
                2fe3bd994b3189899d93f1d5a881e725e046fdc2
            PREFIX
                ${PROJECT_BINARY_DIR}/googletest
            INSTALL_DIR
                ${PROJECT_BINARY_DIR}/temp_install
            CMAKE_ARGS
                -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                $<$<PLATFORM_ID:Windows>:-Dgtest_force_shared_crt:BOOL=ON>
            BUILD_COMMAND
                COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config Release --target install
                COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config Debug --target install
            INSTALL_COMMAND
                ""
            )
        set(GTEST_ROOT ${PROJECT_BINARY_DIR}/temp_install CACHE INTERNAL "")
        set(GMOCK_ROOT ${PROJECT_BINARY_DIR}/temp_install CACHE INTERNAL "")
        set(UAGENT_USE_INTERNAL_GTEST ON)
        list(APPEND _deps googletest)
    endif()
endif()

# Main project.
ExternalProject_Add(uagent
    SOURCE_DIR
        ${PROJECT_SOURCE_DIR}
    BINARY_DIR
        ${CMAKE_CURRENT_BINARY_DIR}
    CMAKE_CACHE_ARGS
        -DUAGENT_SUPERBUILD:BOOL=OFF
        -DUAGENT_USE_INTERNAL_GTEST:BOOL=${UAGENT_USE_INTERNAL_GTEST}
    INSTALL_COMMAND
        ""
    DEPENDS
        ${_deps}
    )
