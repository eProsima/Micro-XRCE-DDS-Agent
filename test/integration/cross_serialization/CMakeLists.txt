###############################################################################
#
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
#
###############################################################################

if(NOT ((MSVC OR MSVC_IDE) AND EPROSIMA_INSTALLER))

    project(cross-serialization-intest-agent CXX)
    set(SRCS AgentSerialization.cpp)
    add_library(${PROJECT_NAME} STATIC ${SRCS})

    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            microxrcedds_agent
            $<$<BOOL:${UAGENT_LOGGER_PROFILE}>:spdlog::spdlog>
        )
    get_target_property(MICROXRCEDDS_AGENT_DIR microxrcedds_agent SOURCE_DIR)
    target_include_directories(${PROJECT_NAME}
        PUBLIC
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
        PRIVATE
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
            $<BUILD_INTERFACE:${MICROXRCEDDS_AGENT_DIR}/src/cpp>
        )

    set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        )

endif()
