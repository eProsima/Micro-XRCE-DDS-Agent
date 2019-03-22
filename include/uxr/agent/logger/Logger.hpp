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
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#endif

namespace eprosima {
namespace uxr {
namespace logger {

#ifdef PROFILE_LOGGER
#define UXR_AGENT_LOG_TRACE(...) SPDLOG_TRACE(__VA_ARGS__)
#else
#define UXR_AGENT_LOG_TRACE(...) void(0)
#endif

#ifdef PROFILE_LOGGER
#define UXR_AGENT_LOG_DEBUG(...) SPDLOG_DEBUG(__VA_ARGS__)
#else
#define UXR_AGENT_LOG_DEBUG(...) void(0)
#endif

#ifdef PROFILE_LOGGER
#define UXR_AGENT_LOG_INFO(...) SPDLOG_INFO(__VA_ARGS__)
#else
#define UXR_AGENT_LOG_INFO(...) void(0)
#endif

#ifdef PROFILE_LOGGER
#define UXR_AGENT_LOG_WARN(...) SPDLOG_WARN(__VA_ARGS__)
#else
#define UXR_AGENT_LOG_WARN(...) (void)0
#endif

#ifdef PROFILE_LOGGER
#define UXR_AGENT_LOG_ERROR(...) SPDLOG_ERROR(__VA_ARGS__)
#else
#define UXR_AGENT_LOG_ERROR(...) (void)0
#endif

#ifdef PROFILE_LOGGER
#define UXR_AGENT_LOG_CRITICAL(...) SPDLOG_CRITICAL(__VA_ARGS__)
#else
#define UXR_AGENT_LOG_CRITICAL(...) (void)0
#endif

} // namespace logger
} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_LOGGER_LOGGER_HPP_
