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
    find_package(microxrcedds_client "1.1.2" EXACT QUIET)
    if(NOT microxrcedds_client_FOUND)
        ExternalProject_Add(uclient
            GIT_REPOSITORY
                https://github.com/eProsima/Micro-XRCE-DDS-Client.git
            GIT_TAG
                v1.1.2
            PREFIX
                ${PROJECT_BINARY_DIR}/uclient
            INSTALL_DIR
                ${PROJECT_BINARY_DIR}/temp_install
            CMAKE_CACHE_ARGS
                -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
                -DBUILD_SHARED_LIBS:BOOL=ON
                -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
            )
        list(APPEND _deps uclient)
    endif()
endif()

# Fast CDR.
unset(fastcdr_DIR CACHE)
find_package(fastcdr "1.0.9" EXACT QUIET)
if(NOT fastcdr_FOUND)
    ExternalProject_Add(fastcdr
        GIT_REPOSITORY
            https://github.com/eProsima/Fast-CDR.git
        GIT_TAG
            v1.0.9
        PREFIX
            ${PROJECT_BINARY_DIR}/fastcdr
        INSTALL_DIR
            ${PROJECT_BINARY_DIR}/temp_install
        CMAKE_CACHE_ARGS
            -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
            -DBUILD_SHARED_LIBS:BOOL=ON
            -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        )
    list(APPEND _deps fastcdr)
endif()

if(UAGENT_FAST_PROFILE)
    # Fast RTPS.
    unset(fastrtps_DIR CACHE)
    find_package(fastrtps "1.8.0" EXACT QUIET)
    if(NOT fastrtps_FOUND)
        ExternalProject_Add(fastrtps
            GIT_REPOSITORY
                https://github.com/eProsima/Fast-RTPS.git
            GIT_TAG
                v1.8.0
            PREFIX
                ${PROJECT_BINARY_DIR}/fastrtps
            INSTALL_DIR
                ${PROJECT_BINARY_DIR}/temp_install
            CMAKE_CACHE_ARGS
                -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
                -DBUILD_SHARED_LIBS:BOOL=ON
                -DCMAKE_PREFIX_PATH:PATH="${CMAKE_PREFIX_PATH};${PROJECT_BINARY_DIR}/temp_install"
                -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                -DTHIRDPARTY:BOOL=ON
                -DSECURITY:BOOL=${UAGENT_SECURITY_PROFILE}
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
        GIT_REPOSITORY
            https://github.com/CLIUtils/CLI11.git
        GIT_TAG
            v1.7.1
        PREFIX
            ${PROJECT_BINARY_DIR}/CLI11
        INSTALL_DIR
            ${PROJECT_BINARY_DIR}/temp_install
        CMAKE_CACHE_ARGS
            -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
            -DBUILD_SHARED_LIBS:BOOL=ON
            -DCMAKE_PREFIX_PATH:PATH="${CMAKE_PREFIX_PATH};${CMAKE_INSTALL_PREFIX}"
            -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
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
            GIT_REPOSITORY
                https://github.com/gabime/spdlog.git
            GIT_TAG
                v1.3.1
            PREFIX
                ${PROJECT_BINARY_DIR}/spdlog
            INSTALL_DIR
                ${PROJECT_BINARY_DIR}/temp_install
            CMAKE_CACHE_ARGS
                -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
                -DBUILD_SHARED_LIBS:BOOL=ON
                -DCMAKE_PREFIX_PATH:PATH="${CMAKE_PREFIX_PATH};${CMAKE_INSTALL_PREFIX}"
                -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                -DSPDLOG_BUILD_EXAMPLES:BOOL=OFF
                -DSPDLOG_BUILD_BENCH:BOOL=OFF
                -DSPDLOG_BUILD_TESTS:BOOL=OFF
                -DSPDLOG_INSTALL:BOOL=ON
            )
        list(APPEND _deps spdlog)
    endif()
endif()

# googletest.
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

# sanitizers.
unset(Sanitizers_DIR CACHE)
find_package(Sanitizers QUIET)
if(NOT Sanitizers_FOUND)
    ExternalProject_Add(sanitizers
        GIT_REPOSITORY
            https://github.com/arsenm/sanitizers-cmake
        PREFIX
            ${PROJECT_BINARY_DIR}/sanitizers
        BUILD_COMMAND
            ""
        INSTALL_COMMAND
            ""
        )
    ExternalProject_Get_Property(sanitizers SOURCE_DIR)
    set(SANITIZERS_ROOT ${SOURCE_DIR} CACHE INTERNAL "")
    list(APPEND _deps sanitizers)
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
