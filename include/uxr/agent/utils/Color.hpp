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

#define UXR_COLOR_RESET         "\033[m"
#define UXR_COLOR_BOLD          "\033[1m"
#define UXR_COLOR_DARK          "\033[2m"
#define UXR_COLOR_UNDERLINE     "\033[4m"
#define UXR_COLOR_BLINK         "\033[5m"
#define UXR_COLOR_REVERSE       "\033[7m"
#define UXR_COLOR_CONCEALED     "\033[8m"
#define UXR_COLOR_CLEAR_LINE    "\033[K"

#define UXR_COLOR_BLACK         "\033[30m"
#define UXR_COLOR_RED           "\033[31m"
#define UXR_COLOR_GREEN         "\033[32m"
#define UXR_COLOR_YELLOW        "\033[33m"
#define UXR_COLOR_BLUE          "\033[34m"
#define UXR_COLOR_MAGENTA       "\033[35m"
#define UXR_COLOR_CYAN          "\033[36m"
#define UXR_COLOR_WHITE         "\033[37m"

#define UXR_COLOR_ON_BLACK      "\033[40m"
#define UXR_COLOR_ON_RED        "\033[41m"
#define UXR_COLOR_ON_GREEN      "\033[42m"
#define UXR_COLOR_ON_YELLOW     "\033[43m"
#define UXR_COLOR_ON_BLUE       "\033[44m"
#define UXR_COLOR_ON_MAGENTA    "\033[45m"
#define UXR_COLOR_ON_CYAN       "\033[46m"
#define UXR_COLOR_ON_WHITE      "\033[47m"

#ifdef _WIN32
#define UXR_DECORATE_COLOR(MESSAGE, COLOR_NAME)    MESSAGE
#else
#define UXR_DECORATE_COLOR(MESSAGE, COLOR_NAME)    UXR_COLOR_ ## COLOR_NAME MESSAGE UXR_COLOR_RESET
#endif

#define UXR_DECORATE_BLACK(MESSAGE)     UXR_DECORATE_COLOR(MESSAGE, BLACK)
#define UXR_DECORATE_RED(MESSAGE)       UXR_DECORATE_COLOR(MESSAGE, RED)
#define UXR_DECORATE_GREEN(MESSAGE)     UXR_DECORATE_COLOR(MESSAGE, GREEN)
#define UXR_DECORATE_YELLOW(MESSAGE)    UXR_DECORATE_COLOR(MESSAGE, YELLOW)
#define UXR_DECORATE_BLUE(MESSAGE)      UXR_DECORATE_COLOR(MESSAGE, BLUE)
#define UXR_DECORATE_MAGENTA(MESSAGE)   UXR_DECORATE_COLOR(MESSAGE, MAGENTA)
#define UXR_DECORATE_CYAN(MESSAGE)      UXR_DECORATE_COLOR(MESSAGE, CYAN)
#define UXR_DECORATE_WHITE(MESSAGE)     UXR_DECORATE_COLOR(MESSAGE, WHITE)

#define UXR_DECORATE_ON_BLACK(MESSAGE)     UXR_DECORATE_COLOR(MESSAGE, ON_BLACK)
#define UXR_DECORATE_ON_RED(MESSAGE)       UXR_DECORATE_COLOR(MESSAGE, ON_RED)
#define UXR_DECORATE_ON_GREEN(MESSAGE)     UXR_DECORATE_COLOR(MESSAGE, ON_GREEN)
#define UXR_DECORATE_ON_YELLOW(MESSAGE)    UXR_DECORATE_COLOR(MESSAGE, ON_YELLOW)
#define UXR_DECORATE_ON_BLUE(MESSAGE)      UXR_DECORATE_COLOR(MESSAGE, ON_BLUE)
#define UXR_DECORATE_ON_MAGENTA(MESSAGE)   UXR_DECORATE_COLOR(MESSAGE, ON_MAGENTA)
#define UXR_DECORATE_ON_CYAN(MESSAGE)      UXR_DECORATE_COLOR(MESSAGE, ON_CYAN)
#define UXR_DECORATE_ON_WHITE(MESSAGE)     UXR_DECORATE_COLOR(MESSAGE, ON_WHITE)

#endif // XRC_UTILS_COLOR_HPP_
