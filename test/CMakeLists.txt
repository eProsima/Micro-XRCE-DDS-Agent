# Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

if(NOT ((MSVC OR MSVC_IDE) AND EPROSIMA_INSTALLER))
    include(${PROJECT_SOURCE_DIR}/cmake/dev/gtest.cmake)

    # If Windows, link against export dll functions
    if(WIN32)
        add_definitions(-DFASTCDR_DYN_LINK)
    endif()

    ###############################################################################
    # Testing libraries
    ###############################################################################
    add_library(testcommon STATIC Common.cpp)
    target_link_libraries(testcommon agent)


    ###############################################################################
    # Unit tests
    ###############################################################################

    # Utils test.
    set(UTILS_SOURCE UtilsTests.cpp)
    add_executable(UtilsTest ${UTILS_SOURCE})
    add_gtest(UtilsTest ${UTILS_SOURCE})
    target_include_directories(UtilsTest PRIVATE ${GTEST_INCLUDE_DIRS})
    target_link_libraries(UtilsTest testcommon ${GTEST_BOTH_LIBRARIES})

    # XRCE test.
    set(XRCETESTS_SOURCE XRCETests.cxx)
    add_executable(XRCETest ${XRCETESTS_SOURCE})
    add_gtest(XRCETest ${XRCETESTS_SOURCE})
    target_include_directories(XRCETest PRIVATE ${GTEST_INCLUDE_DIRS})
    target_link_libraries(XRCETest testcommon ${GTEST_BOTH_LIBRARIES})

    file(COPY ${PROJECT_SOURCE_DIR}/DEFAULT_FASTRTPS_PROFILES.xml
        DESTINATION ${PROJECT_BINARY_DIR}/test/Release
    )

    # Agent test.
    # TODO (julian): fix DataReaderTests.
    set(AGENTTESTS_SOURCE AgentTests.cpp DataReaderTests.cpp)
    add_executable(AgentTest ${AGENTTESTS_SOURCE})
    add_gtest(AgentTest ${AGENTTESTS_SOURCE})
    target_include_directories(AgentTest PRIVATE ${GTEST_INCLUDE_DIRS})
    target_link_libraries(AgentTest testcommon ${GTEST_BOTH_LIBRARIES})

    if(WIN32)
        string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
	set_property(TEST AgentTest APPEND PROPERTY ENVIRONMENT
            "PATH=$<SHELL_PATH:$<TARGET_FILE_DIR:transport>\\;$<TARGET_FILE_DIR:fastrtps>\\;
	          $<TARGET_FILE_DIR:fastcdr>>\\;${WIN_PATH}"
        )
    endif()

    set(AGENTINTEGRATIONTESTS_SOURCE integration/AgenIntegrationTests.cpp)
    add_executable(AgentIntegrationTest ${AGENTINTEGRATIONTESTS_SOURCE})
    add_gtest(AgentIntegrationTest ${AGENTINTEGRATIONTESTS_SOURCE})
    target_include_directories(AgentIntegrationTest PRIVATE ${GTEST_INCLUDE_DIRS})
    target_link_libraries(AgentIntegrationTest agent transport fastcdr fastrtps ${GTEST_BOTH_LIBRARIES})

    install(TARGETS AgentIntegrationTest
        EXPORT ${PROJECT_NAME}Targets
        RUNTIME DESTINATION ${BIN_INSTALL_DIR}
        LIBRARY DESTINATION ${LIB_INSTALL_DIR}
        ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
        COMPONENT libraries
    )

endif()
