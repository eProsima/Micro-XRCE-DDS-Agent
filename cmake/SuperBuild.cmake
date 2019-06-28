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
    find_package(microxrcedds_client ${_micro_xrce_dds_client_version}  EXACT QUIET)
    if(NOT microxrcedds_client_FOUND)
        ExternalProject_Add(uclient
            GIT_REPOSITORY
                https://github.com/eProsima/Micro-XRCE-DDS-Client.git
            GIT_TAG
                v1.1.3
            PREFIX
                ${PROJECT_BINARY_DIR}/uclient
            INSTALL_DIR
                ${PROJECT_BINARY_DIR}/temp_install/uclient
            CMAKE_CACHE_ARGS
                -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
                -DBUILD_SHARED_LIBS:BOOL=ON
                -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                -DCMAKE_TOOLCHAIN_FILE:PATH=${CMAKE_TOOLCHAIN_FILE}
            )
        list(APPEND _deps uclient)
    endif()
endif()

# Fast CDR.
unset(fastcdr_DIR CACHE)
find_package(fastcdr ${_fast_cdr_version} EXACT QUIET)
if(NOT fastcdr_FOUND)
    ExternalProject_Add(fastcdr
        GIT_REPOSITORY
            https://github.com/eProsima/Fast-CDR.git
        GIT_TAG
            v1.0.11
        PREFIX
            ${PROJECT_BINARY_DIR}/fastcdr
        INSTALL_DIR
            ${PROJECT_BINARY_DIR}/temp_install/fastcdr
        CMAKE_CACHE_ARGS
            -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
            -DBUILD_SHARED_LIBS:BOOL=ON
            -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
            -DCMAKE_TOOLCHAIN_FILE:PATH=${CMAKE_TOOLCHAIN_FILE}
        PATCH_COMMAND
            COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/src/cpp/CMakeLists.txt <SOURCE_DIR>/src/cpp/CMakeLists.txt.bak
            COMMAND ${CMAKE_COMMAND} -DSOVERSION_FILE=<SOURCE_DIR>/src/cpp/CMakeLists.txt -P ${PROJECT_SOURCE_DIR}/cmake/Soversion.cmake
        TEST_COMMAND
            COMMAND ${CMAKE_COMMAND} -E rename <SOURCE_DIR>/src/cpp/CMakeLists.txt.bak <SOURCE_DIR>/src/cpp/CMakeLists.txt
        )
    list(APPEND _deps fastcdr)
endif()

if(UAGENT_FAST_PROFILE)
    # Fast RTPS.
    unset(fastrtps_DIR CACHE)
    find_package(fastrtps ${_fast_rtps_version} EXACT QUIET)
    if(NOT fastrtps_FOUND)
        ExternalProject_Add(fastrtps
            GIT_REPOSITORY
                https://github.com/eProsima/Fast-RTPS.git
            GIT_TAG
                v1.8.2
            PREFIX
                ${PROJECT_BINARY_DIR}/fastrtps
            INSTALL_DIR
                ${PROJECT_BINARY_DIR}/temp_install/fastrtps
            CMAKE_CACHE_ARGS
                -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
                -DCMAKE_PREFIX_PATH:PATH="${CMAKE_PREFIX_PATH};${PROJECT_BINARY_DIR}/temp_install"
                -DBUILD_SHARED_LIBS:BOOL=ON
                -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                -DCMAKE_TOOLCHAIN_FILE:PATH=${CMAKE_TOOLCHAIN_FILE}
                -DTHIRDPARTY:BOOL=ON
                -DSECURITY:BOOL=${UAGENT_SECURITY_PROFILE}
            DEPENDS
                fastcdr
            PATCH_COMMAND
                COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/src/cpp/CMakeLists.txt <SOURCE_DIR>/src/cpp/CMakeLists.txt.bak
                COMMAND ${CMAKE_COMMAND} -DSOVERSION_FILE=<SOURCE_DIR>/src/cpp/CMakeLists.txt -P ${PROJECT_SOURCE_DIR}/cmake/Soversion.cmake
            TEST_COMMAND
                COMMAND ${CMAKE_COMMAND} -E rename <SOURCE_DIR>/src/cpp/CMakeLists.txt.bak <SOURCE_DIR>/src/cpp/CMakeLists.txt
            )
        list(APPEND _deps fastrtps)
    endif()
endif()

# CLI11.
unset(CLI11_DIR CACHE)
find_package(CLI11 ${_cli11_version} EXACT QUIET)
if(NOT CLI11_FOUND)
    ExternalProject_Add(cli11
        GIT_REPOSITORY
            https://github.com/CLIUtils/CLI11.git
        GIT_TAG
            v1.7.1
        PREFIX
            ${PROJECT_BINARY_DIR}/CLI11
        INSTALL_DIR
            ${PROJECT_BINARY_DIR}/temp_install/cli11
        CMAKE_CACHE_ARGS
            -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
            -DCMAKE_PREFIX_PATH:PATH="${CMAKE_PREFIX_PATH};${CMAKE_INSTALL_PREFIX}"
            -DBUILD_SHARED_LIBS:BOOL=ON
            -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
            -DCMAKE_TOOLCHAIN_FILE:PATH=${CMAKE_TOOLCHAIN_FILE}
            -DCLI11_TESTING:BOOL=OFF
            -DCLI11_EXAMPLES:BOOL=OFF
        )
    list(APPEND _deps cli11)
endif()

if(UAGENT_LOGGER_PROFILE)
    # spdlog.
    unset(spdlog_DIR CACHE)
    find_package(spdlog ${_spdlog_version} EXACT QUIET)
    if(NOT spdlog_FOUND)
        ExternalProject_Add(spdlog
            GIT_REPOSITORY
                https://github.com/gabime/spdlog.git
            GIT_TAG
                v1.3.1
            PREFIX
                ${PROJECT_BINARY_DIR}/spdlog
            INSTALL_DIR
                ${PROJECT_BINARY_DIR}/temp_install/spdlog
            CMAKE_CACHE_ARGS
                -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
                -DCMAKE_PREFIX_PATH:PATH="${CMAKE_PREFIX_PATH};${CMAKE_INSTALL_PREFIX}"
                -DBUILD_SHARED_LIBS:BOOL=ON
                -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                -DCMAKE_TOOLCHAIN_FILE:PATH=${CMAKE_TOOLCHAIN_FILE}
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
                ${PROJECT_BINARY_DIR}/temp_install/googletest
            CMAKE_ARGS
                -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                -DCMAKE_TOOLCHAIN_FILE:PATH=${CMAKE_TOOLCHAIN_FILE}
                $<$<PLATFORM_ID:Windows>:-Dgtest_force_shared_crt:BOOL=ON>
            BUILD_COMMAND
                COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config Release --target install
                COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config Debug --target install
            INSTALL_COMMAND
                ""
            )
        set(GTEST_ROOT ${PROJECT_BINARY_DIR}/temp_install/googletest CACHE INTERNAL "")
        set(GMOCK_ROOT ${PROJECT_BINARY_DIR}/temp_install/googletest CACHE INTERNAL "")
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

set(UAGENT_DEPENDS "${_deps}" CACHE INTERNAL "")
