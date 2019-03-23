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
#include <spdlog/fmt/bin_to_hex.h>
#include <spdlog/sinks/stdout_sinks.h>
#endif

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

#ifdef PROFILE_LOGGER
#define UXR_AGENT_LOG_TO_HEX(...) spdlog::to_hex(__VA_ARGS__)
#else
#define UXR_AGENT_LOG_HEX(...) void(0)
#endif

#ifdef PROFILE_LOGGER
#define UXR_AGENT_LOG_DEBUG_INPUT_MESSAGE(...) eprosima::uxr::Logger::instance().log_input_message(__VA_ARGS__)
#else
#define UXR_AGENT_LOG_INPUT_MESSAGE(...) void(0)
#endif

#ifdef PROFILE_LOGGER
#define UXR_AGENT_LOG_DEBUG_OUTPUT_MESSAGE(...) eprosima::uxr::Logger::instance().log_output_message(__VA_ARGS__)
#else
#define UXR_AGENT_LOG_OUTPUT_MESSAGE(...) void(0)
#endif

namespace eprosima {
namespace uxr {

class Logger
{
public:
    static Logger& instance();

    void log_input_message(
            uint32_t client_key,
            uint8_t* buf,
            size_t len);

    void log_output_message(
            uint32_t client_key,
            uint8_t* buf,
            size_t len);
private:
    Logger();
    ~Logger() = default;

    Logger(Logger&&) = delete;
    Logger(const Logger&) = delete;
    Logger operator=(Logger&&) = delete;
    Logger operator=(const Logger&) = delete;

private:
    std::shared_ptr<spdlog::logger> message_logger_;

//class Message;
//
//enum Direction : uint8_t
//{
//    INPUT,
//    OUTPUT
//};
//
//static void message()
//{
//    const std::string reset = "\033[m";
//    const std::string bold = "\033[1m";
//    const std::string dark = "\033[2m";
//    const std::string underline = "\033[4m";
//    const std::string blink = "\033[5m";
//    const std::string reverse = "\033[7m";
//    const std::string concealed = "\033[8m";
//    const std::string clear_line = "\033[K";
//
//    const std::string black = "\033[30m";
//    const std::string red = "\033[31m";
//    const std::string green = "\033[32m";
//    const std::string yellow = "\033[33m";
//    const std::string blue = "\033[34m";
//    const std::string magenta = "\033[35m";
//    const std::string cyan = "\033[36m";
//    const std::string white = "\033[37m";
//
//    /// Background colors
//    const std::string on_black = "\033[40m";
//    const std::string on_red = "\033[41m";
//    const std::string on_green = "\033[42m";
//    const std::string on_yellow = "\033[43m";
//    const std::string on_blue = "\033[44m";
//    const std::string on_magenta = "\033[45m";
//    const std::string on_cyan = "\033[46m";
//    const std::string on_white = "\033[47m";
//
//    auto message_loger = spdlog::default_logger()->clone("<·>");
////    auto color_sink = static_cast<spdlog::sinks::ansicolor_stdout_sink_st*>(message_loger->sinks()[0].get());
//    auto color_sink = static_cast<spdlog::sinks::ansicolor_stdout_sink_st*>(message_loger->sinks()[0].get());
////    color_sink->set_color(spdlog::level::info, "\033[35m");
//
//    color_sink->set_color(spdlog::level::info, blue + bold);
//    message_loger->info("info");
//
//    color_sink->set_color(spdlog::level::info, blue);
//    message_loger->info("info");
//
//    color_sink->set_color(spdlog::level::info, magenta + bold);
//    message_loger->info("info");
//
//    color_sink->set_color(spdlog::level::info, magenta);
//    message_loger->info("info");
//}

};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_LOGGER_LOGGER_HPP_
