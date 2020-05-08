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

cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

set(SOVERSION_FILE "./CMakeLists.txt" CACHE FILEPATH "File path")

file(READ ${SOVERSION_FILE} _input)
string(REGEX REPLACE "(SOVERSION)([ \t\r\n]+)([^ \t\r\n\\)]*)" "\\1 \$\{PROJECT_VERSION\}" _output ${_input})
file(WRITE ${SOVERSION_FILE} ${_output})