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
#include <uxr/agent/utils/Color.hpp>

#ifdef PROFILE_LOGGER
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/bin_to_hex.h>
#include <spdlog/sinks/stdout_sinks.h>
#endif

#define UXR_AGENT_LOG_STATUS "{:<30} | "

#ifdef PROFILE_LOGGER
#define UXR_AGENT_LOG_TRACE(...) SPDLOG_TRACE(UXR_AGENT_LOG_STATUS __VA_ARGS__)
#else
#define UXR_AGENT_LOG_TRACE(...) void(0)
#endif

#ifdef PROFILE_LOGGER
#define UXR_AGENT_LOG_DEBUG(...) SPDLOG_DEBUG(UXR_AGENT_LOG_STATUS __VA_ARGS__)
#else
#define UXR_AGENT_LOG_DEBUG(...) void(0)
#endif

#ifdef PROFILE_LOGGER
#define UXR_AGENT_LOG_INFO(...) SPDLOG_INFO(UXR_AGENT_LOG_STATUS __VA_ARGS__)
#else
#define UXR_AGENT_LOG_INFO(...) void(0)
#endif

#ifdef PROFILE_LOGGER
#define UXR_AGENT_LOG_WARN(...) SPDLOG_WARN(UXR_AGENT_LOG_STATUS __VA_ARGS__)
#else
#define UXR_AGENT_LOG_WARN(...) (void)0
#endif

#ifdef PROFILE_LOGGER
#define UXR_AGENT_LOG_ERROR(...) SPDLOG_ERROR(UXR_AGENT_LOG_STATUS __VA_ARGS__)
#else
#define UXR_AGENT_LOG_ERROR(...) (void)0
#endif

#ifdef PROFILE_LOGGER
#define UXR_AGENT_LOG_CRITICAL(...) SPDLOG_CRITICAL(UXR_AGENT_LOG_STATUS __VA_ARGS__)
#else
#define UXR_AGENT_LOG_CRITICAL(...) (void)0
#endif

#ifdef PROFILE_LOGGER
#define UXR_AGENT_LOG_TO_HEX(...) spdlog::to_hex(__VA_ARGS__)
#else
#define UXR_AGENT_LOG_HEX(...) void(0)
#endif

#ifdef PROFILE_LOGGER
#define UXR_AGENT_LOG_MESSAGE(BUF, LEN, FORMAT, ...) \
    if (spdlog::default_logger()->should_log(spdlog::level::trace)) \
    { \
        SPDLOG_DEBUG(UXR_AGENT_LOG_STATUS FORMAT "{}", __VA_ARGS__, LEN, spdlog::to_hex(BUF, BUF + LEN)); \
    } \
    else \
    { \
        SPDLOG_DEBUG(UXR_AGENT_LOG_STATUS FORMAT, __VA_ARGS__, LEN); \
    } \
    void(0)
#else
#define UXR_AGENT_LOG_MESSAGE(...) void(0)
#endif

namespace eprosima {
namespace uxr {
namespace logger {

inline std::string status_ok(const std::string& message)
{
    return color::green + message + color::reset;
}

inline std::string status_warning(const std::string& message)
{
    return  color::yellow + message + color::reset;
}

inline std::string status_error(const std::string& message)
{
    return  color::red + message + color::reset;
}

inline std::string status_info(const std::string& message)
{
    return  color::white + message + color::reset;
}

} // namespace logger
} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_LOGGER_LOGGER_HPP_
