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

#ifndef UXR_AGENT_LOGGER_LOGGER_HPP_
#define UXR_AGENT_LOGGER_LOGGER_HPP_

#include <uxr/agent/config.hpp>

#ifdef PROFILE_LOGGER
#include <spdlog/spdlog.h>
#endif

#include <typeinfo>

namespace eprosima {
namespace uxr {
namespace logger {

template<typename... Args>
inline void sink(Args const& ...) {}

template<typename... Args>
inline void trace(const char* fmt, const Args&... args)
{
#ifdef PROFILE_LOGGER
    spdlog::trace(fmt, args...);
#else
    (void) fmt;
    sink(args...);
#endif
}

template<typename... Args>
inline void debug(const char* fmt, const Args&... args)
{
#ifdef PROFILE_LOGGER
    spdlog::debug(fmt, args...);
#else
    (void) fmt;
    sink(args...);
#endif
}

template<typename... Args>
inline void info(const char* fmt, const Args&... args)
{
#ifdef PROFILE_LOGGER
    spdlog::info(fmt, args...);
#else
    (void) fmt;
    sink(args...);
#endif
}

template<typename... Args>
inline void warn(const char* fmt, const Args&... args)
{
#ifdef PROFILE_LOGGER
    spdlog::warn(fmt, args...);
#else
    (void) fmt;
    sink(args...);
#endif
}

template<typename... Args>
inline void error(const char* fmt, const Args&... args)
{
#ifdef PROFILE_LOGGER
    spdlog::error(fmt, args...);
#else
    (void) fmt;
    sink(args...);
#endif
}

template<typename... Args>
inline void critical(const char* fmt, const Args&... args)
{
#ifdef PROFILE_LOGGER
    spdlog::critical(fmt, args...);
#else
    (void) fmt;
    sink(args...);
#endif
}

} // namespace logger
} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_LOGGER_LOGGER_HPP_
