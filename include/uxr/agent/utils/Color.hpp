// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef XRC_UTILS_COLOR_HPP_
#define XRC_UTILS_COLOR_HPP_

#include <string>

namespace eprosima {
namespace uxr {
namespace color {

const std::string reset = "\033[m";
const std::string bold = "\033[1m";
const std::string dark = "\033[2m";
const std::string underline = "\033[4m";
const std::string blink = "\033[5m";
const std::string reverse = "\033[7m";
const std::string concealed = "\033[8m";
const std::string clear_line = "\033[K";

const std::string black = "\033[30m";
const std::string red = "\033[31m";
const std::string green = "\033[32m";
const std::string yellow = "\033[33m";
const std::string blue = "\033[34m";
const std::string magenta = "\033[35m";
const std::string cyan = "\033[36m";
const std::string white = "\033[37m";

const std::string on_black = "\033[40m";
const std::string on_red = "\033[41m";
const std::string on_green = "\033[42m";
const std::string on_yellow = "\033[43m";
const std::string on_blue = "\033[44m";
const std::string on_magenta = "\033[45m";
const std::string on_cyan = "\033[46m";
const std::string on_white = "\033[47m";

} // namespace color
} // namespace uxr
} // namespace eprosima

#endif // XRC_UTILS_COLOR_HPP_
