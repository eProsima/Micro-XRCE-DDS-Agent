# Copyright 2021-present Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
if (UAGENT_BUILD_USAGE_EXAMPLES)
    add_executable(CustomXRCEAgent custom_agent.cpp)
    target_link_libraries(CustomXRCEAgent
        PRIVATE
            ${PROJECT_NAME}
            $<$<BOOL:$<PLATFORM_ID:Linux>>:rt>
            $<$<BOOL:$<PLATFORM_ID:Linux>>:dl>
        )

    set_target_properties(CustomXRCEAgent
        PROPERTIES
            CXX_STANDARD
                11
            CXX_STANDARD_REQUIRED
                YES
        )
endif()