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

#ifdef UAGENT_LOGGER_PROFILE
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/bin_to_hex.h>
#include <spdlog/sinks/stdout_sinks.h>
#endif

#ifdef _WIN32
#define UXR_LOG_PATTERN "[%E.%f]" \
                        " %^%-8l%$ | " \
                        "%-18s" " | " \
                        "%-24!" " | " \
                        "%v"
#else
#define UXR_LOG_PATTERN UXR_COLOR_MAGENTA "[%E.%f]" UXR_COLOR_RESET \
                        " %^%-8l%$ | " \
                        UXR_COLOR_BLUE "%-18s" UXR_COLOR_RESET  " | " \
                        UXR_COLOR_WHITE "%-24!" UXR_COLOR_RESET " | " \
                        "%v"
#endif

#define UXR_CLIENT_KEY_STR      "client_key"
#define UXR_FILE_FD_STR         "fd"
#define UXR_SESSION_ID_STR      "session_id"
#define UXR_OBJECT_ID_STR       "object_id"
#define UXR_PARTICIPANT_ID_STR  "participant_id"
#define UXR_TOPIC_ID_STR        "topic_id"
#define UXR_PUBLISHER_ID_STR    "publisher_id"
#define UXR_SUBSCRIBER_ID_STR   "subscriber_id"
#define UXR_DATAWRITER_ID_STR   "datawriter_id"
#define UXR_DATAREADER_ID_STR   "datareader_id"
#define UXR_REQUESTER_ID_STR    "requester_id"
#define UXR_REPLIER_ID_STR      "replier_id"
#define UXR_LEN_STR             "len"
#define UXR_DATA_STR            "data"

#define UXR_CLIENT_KEY_FORMAT       "0x{:08X}"
#define UXR_FILE_FD_FORMAT          "{}"
#define UXR_SESSION_ID_FORMAT       "0x{:02X}"
#define UXR_OBJECT_ID_FORMAT        "0x{:04X}"
#define UXR_PARTICIPANT_ID_FORMAT   "0x{:03X}(1)"
#define UXR_TOPIC_ID_FORMAT         "0x{:03X}(2)"
#define UXR_PUBLISHER_ID_FORMAT     "0x{:03X}(3)"
#define UXR_SUBSCRIBER_ID_FORMAT    "0x{:03X}(4)"
#define UXR_DATAWRITER_ID_FORMAT    "0x{:03X}(5)"
#define UXR_DATAREADER_ID_FORMAT    "0x{:03X}(6)"
#define UXR_REQUESTER_ID_FORMAT     "0x{:03X}(7)"
#define UXR_REPLIER_ID_FORMAT       "0x{:03X}(8)"
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
#define UXR_CREATE_REQUESTER_PATTERN    UXR_CREATE_FORMAT_BASE(REQUESTER_ID)    UXR_ADD_FIELD(PARTICIPANT_ID)
#define UXR_CREATE_REPLIER_PATTERN      UXR_CREATE_FORMAT_BASE(REPLIER_ID)      UXR_ADD_FIELD(PARTICIPANT_ID)
#define UXR_MESSAGE_PATTERN             UXR_FIELD(CLIENT_KEY)                   UXR_ADD_FIELD(LEN)
#define UXR_MESSAGE_WITH_DATA_PATTERN   UXR_MESSAGE_PATTERN                     UXR_ADD_FIELD(DATA)
#define UXR_MESSAGE_WITH_FD_PATTERN     UXR_CREATE_FORMAT_BASE(FILE_FD)         UXR_ADD_FIELD(LEN)      UXR_ADD_FIELD(DATA)



#ifdef UAGENT_LOGGER_PROFILE
#define UXR_AGENT_LOG_TRACE(X, Y, ...) SPDLOG_TRACE(UXR_STATUS_FORMAT Y, X, __VA_ARGS__)
#else
#define UXR_AGENT_LOG_TRACE(...) void(0)
#endif

#ifdef UAGENT_LOGGER_PROFILE
#define UXR_AGENT_LOG_DEBUG(X, Y, ...) SPDLOG_DEBUG(UXR_STATUS_FORMAT Y, X, __VA_ARGS__)
#else
#define UXR_AGENT_LOG_DEBUG(...) void(0)
#endif

#ifdef UAGENT_LOGGER_PROFILE
#define UXR_AGENT_LOG_INFO(X, Y, ...) SPDLOG_INFO(UXR_STATUS_FORMAT Y, X, __VA_ARGS__)
#else
#define UXR_AGENT_LOG_INFO(...) void(0)
#endif

#ifdef UAGENT_LOGGER_PROFILE
#define UXR_AGENT_LOG_WARN(X, Y, ...) SPDLOG_WARN(UXR_STATUS_FORMAT Y, X, __VA_ARGS__)
#else
#define UXR_AGENT_LOG_WARN(...) (void)0
#endif

#ifdef UAGENT_LOGGER_PROFILE
#define UXR_AGENT_LOG_ERROR(X, Y, ...) SPDLOG_ERROR(UXR_STATUS_FORMAT Y, X, __VA_ARGS__)
#else
#define UXR_AGENT_LOG_ERROR(...) (void)0
#endif

#ifdef UAGENT_LOGGER_PROFILE
#define UXR_AGENT_LOG_CRITICAL(X, Y, ...) SPDLOG_CRITICAL(UXR_STATUS_FORMAT Y, X, __VA_ARGS__); std::exit(EXIT_FAILURE)
#else
#define UXR_AGENT_LOG_CRITICAL(...) std::exit(EXIT_FAILURE)
#endif

#ifdef UAGENT_LOGGER_PROFILE
#define UXR_AGENT_LOG_TO_HEX(...) spdlog::to_hex(__VA_ARGS__)
#else
#define UXR_AGENT_LOG_HEX(...) void(0)
#endif

#ifdef UAGENT_LOGGER_PROFILE
#define UXR_AGENT_LOG_MESSAGE(STATUS, CLIENT_KEY, BUF, LEN) \
    if (spdlog::default_logger()->should_log(spdlog::level::trace)) \
    { \
        UXR_AGENT_LOG_DEBUG(STATUS, UXR_MESSAGE_WITH_DATA_PATTERN, CLIENT_KEY, LEN, spdlog::to_hex(BUF, BUF + LEN)); \
    } \
    else \
    { \
        UXR_AGENT_LOG_DEBUG(STATUS, UXR_MESSAGE_PATTERN, CLIENT_KEY, LEN, spdlog::to_hex(BUF, BUF + LEN)); \
    } \
    void(0)

#define UXR_MULTIAGENT_LOG_MESSAGE(STATUS, CLIENT_KEY, FD, BUF, LEN) \
    if (spdlog::default_logger()->should_log(spdlog::level::trace)) \
    { \
        UXR_AGENT_LOG_DEBUG(STATUS, UXR_MESSAGE_WITH_FD_PATTERN, CLIENT_KEY, FD, LEN, spdlog::to_hex(BUF, BUF + LEN)); \
    } \
    else \
    { \
        UXR_AGENT_LOG_DEBUG(STATUS, UXR_MESSAGE_PATTERN, CLIENT_KEY, FD, LEN, spdlog::to_hex(BUF, BUF + LEN)); \
    } \
    void(0)
#else
#define UXR_AGENT_LOG_MESSAGE(...) void(0)
#define UXR_MULTIAGENT_LOG_MESSAGE(...) void(0)
#endif

#endif // UXR_AGENT_LOGGER_LOGGER_HPP_
