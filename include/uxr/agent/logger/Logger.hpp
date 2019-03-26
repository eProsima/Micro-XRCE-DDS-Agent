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

#define UXR_LOG_PATTERN color::magenta + "[%E.%f]" + color::reset + \
                        " %^%-8l%$ | " + \
                        color::blue +  "%-18s" + color::reset +  " | " + \
                        color::white + "%-24!" + color::reset + " | " \
                        "%v"

#define UXR_CLIENT_KEY_STR      "client_key"
#define UXR_SESSION_ID_STR      "session_id"
#define UXR_OBJECT_ID_STR       "object_id"
#define UXR_PARTICIPANT_ID_STR  "participant_id"
#define UXR_TOPIC_ID_STR        "topic_id"
#define UXR_PUBLISHER_ID_STR    "publisher_id"
#define UXR_SUBSCRIBER_ID_STR   "subscriber_id"
#define UXR_DATAWRITER_ID_STR   "datawriter_id"
#define UXR_DATAREADER_ID_STR   "datareader_id"
#define UXR_LEN_STR             "len"
#define UXR_DATA_STR            "data"

#define UXR_CLIENT_KEY_FORMAT       "0x{:08X}"
#define UXR_SESSION_ID_FORMAT       "0x{:02X}"
#define UXR_OBJECT_ID_FORMAT        "0x{:04X}"
#define UXR_PARTICIPANT_ID_FORMAT   "0x{:03X}(1)"
#define UXR_TOPIC_ID_FORMAT         "0x{:03X}(2)"
#define UXR_PUBLISHER_ID_FORMAT     "0x{:03X}(3)"
#define UXR_SUBSCRIBER_ID_FORMAT    "0x{:03X}(4)"
#define UXR_DATAWRITER_ID_FORMAT    "0x{:03X}(5)"
#define UXR_DATAREADER_ID_FORMAT    "0x{:03X}(6)"
#define UXR_LEN_FORMAT              "{}"
#define UXR_DATA_FORMAT             "{:X}"
#define UXR_STATUS_FORMAT           "{:<30} | "

#define UXR_STR_FORMAT_SEP  ": "
#define UXR_FIELD_SEP       ", "

#define UXR_FIELD(NAME)                 UXR_ ## NAME ## _STR  UXR_STR_FORMAT_SEP  UXR_ ## NAME ## _FORMAT
#define UXR_ADD_FIELD(NAME)             UXR_FIELD_SEP  UXR_FIELD(NAME)
#define UXR_CREATE_FORMAT_BASE(NAME)    UXR_FIELD(CLIENT_KEY)   UXR_ADD_FIELD(NAME)

#define UXR_CLIENT_KEY_PATTERN          UXR_FIELD(CLIENT_KEY)
#define UXR_CREATE_SESSION_PATTERN      UXR_CREATE_FORMAT_BASE(SESSION_ID)
#define UXR_CREATE_OBJECT_PATTERN       UXR_CREATE_FORMAT_BASE(OBJECT_ID)
#define UXR_CREATE_PARTICIPANT_PATTERN  UXR_CREATE_FORMAT_BASE(PARTICIPANT_ID)
#define UXR_CREATE_TOPIC_PATTERN        UXR_CREATE_FORMAT_BASE(TOPIC_ID)        UXR_ADD_FIELD(PARTICIPANT_ID)
#define UXR_CREATE_PUBLISHER_PATTERN    UXR_CREATE_FORMAT_BASE(PUBLISHER_ID)    UXR_ADD_FIELD(PARTICIPANT_ID)
#define UXR_CREATE_SUBSCRIBER_PATTERN   UXR_CREATE_FORMAT_BASE(SUBSCRIBER_ID)   UXR_ADD_FIELD(PARTICIPANT_ID)
#define UXR_CREATE_DATAWRITER_PATTERN   UXR_CREATE_FORMAT_BASE(DATAWRITER_ID)   UXR_ADD_FIELD(PUBLISHER_ID)
#define UXR_CREATE_DATAREADER_PATTERN   UXR_CREATE_FORMAT_BASE(DATAREADER_ID)   UXR_ADD_FIELD(SUBSCRIBER_ID)
#define UXR_MESSAGE_PATTERN             UXR_FIELD(CLIENT_KEY)                   UXR_ADD_FIELD(LEN)
#define UXR_MESSAGE_WITH_DATA_PATTERN   UXR_MESSAGE_PATTERN                     UXR_ADD_FIELD(DATA)


#ifdef PROFILE_LOGGER
#define UXR_AGENT_LOG_TRACE(...) SPDLOG_TRACE(UXR_STATUS_FORMAT __VA_ARGS__)
#else
#define UXR_AGENT_LOG_TRACE(...) void(0)
#endif

#ifdef PROFILE_LOGGER
#define UXR_AGENT_LOG_DEBUG(...) SPDLOG_DEBUG(UXR_STATUS_FORMAT __VA_ARGS__)
#else
#define UXR_AGENT_LOG_DEBUG(...) void(0)
#endif

#ifdef PROFILE_LOGGER
#define UXR_AGENT_LOG_INFO(...) SPDLOG_INFO(UXR_STATUS_FORMAT __VA_ARGS__)
#else
#define UXR_AGENT_LOG_INFO(...) void(0)
#endif

#ifdef PROFILE_LOGGER
#define UXR_AGENT_LOG_WARN(...) SPDLOG_WARN(UXR_STATUS_FORMAT __VA_ARGS__)
#else
#define UXR_AGENT_LOG_WARN(...) (void)0
#endif

#ifdef PROFILE_LOGGER
#define UXR_AGENT_LOG_ERROR(...) SPDLOG_ERROR(UXR_STATUS_FORMAT __VA_ARGS__)
#else
#define UXR_AGENT_LOG_ERROR(...) (void)0
#endif

#ifdef PROFILE_LOGGER
#define UXR_AGENT_LOG_CRITICAL(...) SPDLOG_CRITICAL(UXR_STATUS_FORMAT __VA_ARGS__)
#else
#define UXR_AGENT_LOG_CRITICAL(...) (void)0
#endif

#ifdef PROFILE_LOGGER
#define UXR_AGENT_LOG_TO_HEX(...) spdlog::to_hex(__VA_ARGS__)
#else
#define UXR_AGENT_LOG_HEX(...) void(0)
#endif

#ifdef PROFILE_LOGGER
#define UXR_AGENT_LOG_MESSAGE(BUF, LEN, ...) \
    if (spdlog::default_logger()->should_log(spdlog::level::trace)) \
    { \
        UXR_AGENT_LOG_DEBUG(UXR_MESSAGE_WITH_DATA_PATTERN, __VA_ARGS__, LEN, spdlog::to_hex(BUF, BUF + LEN)); \
    } \
    else \
    { \
        UXR_AGENT_LOG_DEBUG(UXR_MESSAGE_PATTERN, __VA_ARGS__, LEN, spdlog::to_hex(BUF, BUF + LEN)); \
    } \
    void(0)
#else
#define UXR_AGENT_LOG_MESSAGE(...) void(0)
#endif

namespace eprosima {
namespace uxr {
namespace logger {

} // namespace logger
} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_LOGGER_LOGGER_HPP_
