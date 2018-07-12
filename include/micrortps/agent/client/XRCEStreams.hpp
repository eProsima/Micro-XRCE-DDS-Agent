// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _MICRORTPS_AGENT_CLIENT_XRCESTREAMS_HPP_
#define _MICRORTPS_AGENT_CLIENT_XRCESTREAMS_HPP_

#include <micrortps/agent/types/XRCETypes.hpp>
#include <micrortps/agent/utils/SeqNum.hpp>

#include <mutex>
#include <vector>
#include <map>
#include <limits>

// TODO (julian): move to global config.
#define STREAM_HISTORY_DEPTH 16
#define SEQ_NUM_LIMITS (1 << 16)

namespace eprosima {
namespace micrortps {

typedef struct XrceMessage
{
    char* buf;
    size_t len;

} XrceMessage;

/******************************************************************************
 * Best-Effort Streams.
 ******************************************************************************/
class BestEffortStream
{
public:
    BestEffortStream() : last_handled_(0xFFFF) {}

    void update(SeqNum seq_num);
    SeqNum get_last_handled() const { return last_handled_; }

private:
    SeqNum last_handled_;
};

/******************************************************************************
 * Reliable Input Stream.
 ******************************************************************************/
class ReliableInputStream
{
public:
    ReliableInputStream() : last_handled_(0xFFFF), last_announced_(0xFFFF) {}

    ReliableInputStream(const ReliableInputStream&) = delete;
    ReliableInputStream& operator=(const ReliableInputStream) = delete;
    ReliableInputStream(ReliableInputStream&&);
    ReliableInputStream& operator=(ReliableInputStream&&);

    void insert_message(SeqNum index, const char* buf, size_t len);
    bool message_available();
    XrceMessage get_next_message();
    void update_from_heartbeat(SeqNum first_available, SeqNum last_available);
    void update_from_message(SeqNum seq_num);
    SeqNum get_first_unacked() const;
    std::array<uint8_t, 2> get_nack_bitmap();

private:
    SeqNum last_handled_;
    SeqNum last_announced_;
    std::map<uint16_t, std::vector<uint8_t>> messages_;
    std::mutex mtx_;
};

/******************************************************************************
 * Reliable Output Stream.
 ******************************************************************************/
class ReliableOutputStream
{
public:
    ReliableOutputStream() : last_sent_(0xFFFF), last_acknown_(0xFFFF) {}

    ReliableOutputStream(const ReliableOutputStream&) = delete;
    ReliableOutputStream& operator=(const ReliableOutputStream) = delete;
    ReliableOutputStream(ReliableOutputStream&&);
    ReliableOutputStream& operator=(ReliableOutputStream&&);


    void push_message(const char* buf, size_t len);
    XrceMessage get_message(SeqNum index);
    void update_from_acknack(SeqNum first_unacked);
    SeqNum get_first_available() { return last_acknown_ + 1;}
    SeqNum get_last_available() { return last_sent_; }
    SeqNum next_message() { return last_sent_ + 1; }
    bool message_pending() { return messages_.size() != 0; }

private:
    SeqNum last_sent_;
    SeqNum last_acknown_;
    std::map<uint16_t, std::vector<uint8_t>> messages_;
    std::mutex mtx_;
};

} // namespace micrortps
} // namespace eprosima

#endif //_MICRORTPS_AGENT_CLIENT_XRCESTREAMS_HPP_
