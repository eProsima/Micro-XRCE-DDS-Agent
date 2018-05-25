# Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

macro(eprosima_find_package package)

    if(NOT ${package}_FOUND AND NOT (EPROSIMA_INSTALLER AND (MSVC OR MSVC_IDE)))

        # Parse arguments.
        set(options REQUIRED)
        cmake_parse_arguments(FIND "${options}" "" "" ${ARGN})

        option(THIRDPARTY_${package} "Activate the use of internal thirdparty ${package}" OFF)

        find_package(${package} QUIET)
        if(NOT ${package}_FOUND AND (THIRDPARTY OR THIRDPARTY_${package}))
            if(NOT EXISTS "${PROJECT_SOURCE_DIR}/thirdparty/${package}/CMakeLists.txt")
                execute_process(
                    COMMAND git submodule update --recursive --init "thirdparty/${package}"
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                    RESULT_VARIABLE EXECUTE_RESULT
                    )
                if(EXECUTE_RESULT EQUAL 0)
                    add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/${package})
                    set(${package}_FOUND TRUE)
                    if(NOT IS_TOP_LEVEL)
                        set(${package}_FOUND TRUE PARENT_SCOPE)
                    endif()
                else()
                    message(WARNING "Cannot configure Git submodule ${package}")
                endif()
            else()
                add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/${package})
                set(${package}_FOUND TRUE)
                if(NOT IS_TOP_LEVEL)
                    set(${package}_FOUND TRUE PARENT_SCOPE)
                endif()
            endif()
        endif()

        if(${package}_FOUND)
            message(STATUS "${package} library found...")
        elseif(${FIND_REQUIRED})
            message(FATAL_ERROR "${package} library not found...")
        else()
            message(STATUS "${package} library not found...")
        endif()
    endif()
endmacro()

macro(eprosima_find_thirdparty package thirdparty_name)
    if(NOT (EPROSIMA_INSTALLER AND (MSVC OR MSVC_IDE)))

        option(THIRDPARTY_${package} "Activate the use of internal thirdparty ${package}" OFF)

        if(THIRDPARTY OR THIRDPARTY_${package})
            execute_process(
                COMMAND git submodule update --recursive --init "thirdparty/${thirdparty_name}"
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                RESULT_VARIABLE EXECUTE_RESULT
                )

            if(EXECUTE_RESULT EQUAL 0)
            else()
                message(FATAL_ERROR "Cannot configure Git submodule ${package}")
            endif()
        endif()

        set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${PROJECT_SOURCE_DIR}/thirdparty/${thirdparty_name})
        set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${PROJECT_SOURCE_DIR}/thirdparty/${thirdparty_name}/${thirdparty_name})
        find_package(${package} REQUIRED QUIET)

    endif()
endmacro()

macro(eprosima_install_libraries)
    if((MSVC OR MSVC_IDE) AND THIRDPARTY AND NOT MINION)
        if(EPROSIMA_INSTALLER)
            # Install includes. Take from x64Win64VS2015
            install(DIRECTORY ${PROJECT_BINARY_DIR}/eprosima_installer/x64Win64VS2015/install/${INCLUDE_INSTALL_DIR}/
                DESTINATION ${INCLUDE_INSTALL_DIR}
                COMPONENT headers
                OPTIONAL
                )

            # Install licenses. Take from x64Win64VS2015
            install(DIRECTORY ${PROJECT_BINARY_DIR}/eprosima_installer/x64Win64VS2015/install/licenses/
                DESTINATION ${LICENSE_INSTALL_DIR}
                COMPONENT licenses
                OPTIONAL
                )
        else()
            if("${CMAKE_BUILD_TYPE}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
                set(BUILD_TYPE_INSTALLATION "Release")
            else()
                set(BUILD_TYPE_INSTALLATION ${CMAKE_BUILD_TYPE})
            endif()

            # Install includes
            install(DIRECTORY ${PROJECT_BINARY_DIR}/external/install/${INCLUDE_INSTALL_DIR}/
                DESTINATION ${INCLUDE_INSTALL_DIR}
                COMPONENT headers
                OPTIONAL
                )

            # Install libraries
            install(DIRECTORY ${PROJECT_BINARY_DIR}/external/install/${BIN_INSTALL_DIR}/
                DESTINATION ${BIN_INSTALL_DIR}
                COMPONENT libraries_${MSVC_ARCH}
                CONFIGURATIONS Debug
                OPTIONAL
                FILES_MATCHING
                PATTERN "*d.*"
                PATTERN "*d-*.*"
                )

            install(DIRECTORY ${PROJECT_BINARY_DIR}/external/install/${BIN_INSTALL_DIR}/
                DESTINATION ${BIN_INSTALL_DIR}
                COMPONENT libraries_${MSVC_ARCH}
                CONFIGURATIONS ${BUILD_TYPE_INSTALLATION}
                OPTIONAL
                FILES_MATCHING
                PATTERN "*"
                PATTERN "*d.*" EXCLUDE
                PATTERN "*d-*.*" EXCLUDE
                )

            install(DIRECTORY ${PROJECT_BINARY_DIR}/external/install/${LIB_INSTALL_DIR}/
                DESTINATION ${LIB_INSTALL_DIR}
                COMPONENT libraries_${MSVC_ARCH}
                CONFIGURATIONS Debug
                OPTIONAL
                FILES_MATCHING
                PATTERN "*d.*"
                PATTERN "*d-*.*"
                PATTERN "*.cmake"
                PATTERN "*-${BUILD_TYPE_INSTALLATION}.cmake" EXCLUDE
                )

            install(DIRECTORY ${PROJECT_BINARY_DIR}/external/install/${LIB_INSTALL_DIR}/
                DESTINATION ${LIB_INSTALL_DIR}
                COMPONENT libraries_${MSVC_ARCH}
                CONFIGURATIONS ${BUILD_TYPE_INSTALLATION}
                OPTIONAL
                FILES_MATCHING
                PATTERN "*"
                PATTERN "*d.*" EXCLUDE
                PATTERN "*d-*.*" EXCLUDE
                PATTERN "*-debug.cmake" EXCLUDE
                )

            # Install licenses
            install(DIRECTORY ${PROJECT_BINARY_DIR}/external/install/licenses/
                DESTINATION ${LICENSE_INSTALL_DIR}
                COMPONENT licenses
                OPTIONAL
                )
        endif()
    elseif(UNIX AND THIRDPARTY AND NOT MINION AND NOT EPROSIMA_INSTALLER)
            # Install includes
            install(DIRECTORY ${PROJECT_BINARY_DIR}/external/install/${INCLUDE_INSTALL_DIR}/
                DESTINATION ${INCLUDE_INSTALL_DIR}
                COMPONENT headers
                OPTIONAL
                )

            # Install libraries
            install(DIRECTORY ${PROJECT_BINARY_DIR}/external/install/${LIB_INSTALL_DIR}/
                DESTINATION ${LIB_INSTALL_DIR}
                USE_SOURCE_PERMISSIONS
                COMPONENT libraries
                )

            # Install licenses
            install(DIRECTORY ${PROJECT_BINARY_DIR}/external/install/licenses/
                DESTINATION ${LICENSE_INSTALL_DIR}
                COMPONENT licenses
                OPTIONAL
                )
    endif()
endmacro()
