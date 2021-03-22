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

enable_language(C)
enable_language(CXX)

if(UAGENT_P2P_PROFILE)
    # Micro XRCE-DDS Client.
    unset(microxrcedds_client_DIR CACHE)
    find_package(microxrcedds_client ${_microxrcedds_client_version} EXACT QUIET)
    if(NOT microxrcedds_client_FOUND)
        ExternalProject_Add(microxrcedds_client
            GIT_REPOSITORY
                https://github.com/eProsima/Micro-XRCE-DDS-Client.git
            GIT_TAG
                ${_microxrcedds_client_tag}
            PREFIX
                ${PROJECT_BINARY_DIR}/microxrcedds_client
            INSTALL_DIR
                ${PROJECT_BINARY_DIR}/temp_install/microxrcedds_client-${_microxrcedds_client_version}
            CMAKE_CACHE_ARGS
                -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
                -DCMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH}
                -DCMAKE_FIND_ROOT_PATH:PATH=${PROJECT_BINARY_DIR}/temp_install
                -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
                -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
                -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
                -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                -DCMAKE_TOOLCHAIN_FILE:PATH=${CMAKE_TOOLCHAIN_FILE}
            )
        list(APPEND _deps microxrcedds_client)
    endif()
endif()

# Fast CDR.
unset(fastcdr_DIR CACHE)
find_package(fastcdr ${_fastcdr_version} EXACT QUIET)
if(NOT fastcdr_FOUND)
    ExternalProject_Add(fastcdr
        GIT_REPOSITORY
            https://github.com/eProsima/Fast-CDR.git
        GIT_TAG
            ${_fastcdr_tag}
        PREFIX
            ${PROJECT_BINARY_DIR}/fastcdr
        INSTALL_DIR
            ${PROJECT_BINARY_DIR}/temp_install/fastcdr-${_fastcdr_version}
        CMAKE_CACHE_ARGS
            -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
            -DCMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH}
            -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
            -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
            -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
            -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
            -DCMAKE_TOOLCHAIN_FILE:PATH=${CMAKE_TOOLCHAIN_FILE}
        UPDATE_COMMAND
            COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/src/cpp/CMakeLists.txt <SOURCE_DIR>/src/cpp/CMakeLists.txt.bak
            COMMAND ${CMAKE_COMMAND} -DSOVERSION_FILE=<SOURCE_DIR>/src/cpp/CMakeLists.txt -P ${PROJECT_SOURCE_DIR}/cmake/Soversion.cmake
        TEST_COMMAND
            COMMAND ${CMAKE_COMMAND} -E rename <SOURCE_DIR>/src/cpp/CMakeLists.txt.bak <SOURCE_DIR>/src/cpp/CMakeLists.txt
        )
    list(APPEND _deps fastcdr)
endif()

if(UAGENT_FAST_PROFILE AND NOT UAGENT_USE_SYSTEM_FASTDDS)
    # Foonathan memory.
    unset(foonathan_memory_DIR CACHE)
    find_package(foonathan_memory QUIET)
    if (NOT foonathan_memory_FOUND)
        ExternalProject_Add(foonathan_memory
            GIT_REPOSITORY
                https://github.com/foonathan/memory.git
            GIT_TAG
                ${_foonathan_memory_tag}
            PREFIX
                ${PROJECT_BINARY_DIR}/foonathan_memory
            INSTALL_DIR
                ${PROJECT_BINARY_DIR}/temp_install/foonathan_memory
            CMAKE_CACHE_ARGS
                -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
                -DFOONATHAN_MEMORY_BUILD_EXAMPLES:BOOL=OFF
                -DFOONATHAN_MEMORY_BUILD_TESTS:BOOL=OFF
                -DFOONATHAN_MEMORY_BUILD_TOOLS:BOOL=ON
                -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
                -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
            )
    endif()

    # Fast DDS.
    unset(fastdds_DIR CACHE)
    find_package(fastrtps ${_fastdds_version} EXACT QUIET)
    if(NOT fastrtps_FOUND)
        ExternalProject_Add(fastdds
            GIT_REPOSITORY
                https://github.com/eProsima/Fast-DDS.git
            GIT_TAG
                ${_fastdds_tag}
            PREFIX
                ${PROJECT_BINARY_DIR}/fastdds
            INSTALL_DIR
                ${PROJECT_BINARY_DIR}/temp_install/fastrtps-${_fastdds_version}
            CMAKE_CACHE_ARGS
                -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
                -DCMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH};${PROJECT_BINARY_DIR}/temp_install
                -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
                -DCMAKE_TOOLCHAIN_FILE:PATH=${CMAKE_TOOLCHAIN_FILE}
                -DCMAKE_FIND_ROOT_PATH:PATH=${PROJECT_BINARY_DIR}/temp_install
                -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
                -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
                -DTHIRDPARTY:BOOL=ON
                -DCOMPILE_TOOLS:BOOL=OFF
                -DSECURITY:BOOL=${UAGENT_SECURITY_PROFILE}
                -DSHM_TRANSPORT_DEFAULT:BOOL=OFF
            DEPENDS
                fastcdr
                foonathan_memory
            UPDATE_COMMAND
                COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/src/cpp/CMakeLists.txt <SOURCE_DIR>/src/cpp/CMakeLists.txt.bak
                COMMAND ${CMAKE_COMMAND} -DSOVERSION_FILE=<SOURCE_DIR>/src/cpp/CMakeLists.txt -P ${PROJECT_SOURCE_DIR}/cmake/Soversion.cmake
            TEST_COMMAND
                COMMAND ${CMAKE_COMMAND} -E rename <SOURCE_DIR>/src/cpp/CMakeLists.txt.bak <SOURCE_DIR>/src/cpp/CMakeLists.txt
            )
        list(APPEND _deps fastdds)
    endif()
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
                ${_spdlog_tag}
            PREFIX
                ${PROJECT_BINARY_DIR}/spdlog
            INSTALL_DIR
                ${PROJECT_BINARY_DIR}/temp_install/spdlog-${_spdlog_version}
            CMAKE_CACHE_ARGS
                -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
                -DCMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH};${CMAKE_INSTALL_PREFIX}
                -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
                -DCMAKE_TOOLCHAIN_FILE:PATH=${CMAKE_TOOLCHAIN_FILE}
                -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
                -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
                -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
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
        CMAKE_ARGS
            $<$<VERSION_GREATER_EQUAL:${CMAKE_VERSION},3.16.3>:-DCMAKE_POLICY_DEFAULT_CMP0077=OLD> # Disable CMP0077 unset warning
        CMAKE_CACHE_ARGS
            -DCMAKE_TOOLCHAIN_FILE:PATH=${CMAKE_TOOLCHAIN_FILE}
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
