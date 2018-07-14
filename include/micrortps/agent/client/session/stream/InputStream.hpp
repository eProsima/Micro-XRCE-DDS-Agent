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

#ifndef _MICRORTPS_AGENT_CLIENT_SESSION_STREAM_INPUT_STREAM_HPP_
#define _MICRORTPS_AGENT_CLIENT_SESSION_STREAM_INPUT_STREAM_HPP_

#include <micrortps/agent/utils/SeqNum.hpp>
#include <micrortps/agent/message/Packet.hpp>
#include <map>

#define MICRORTPS_RELIABLE_STREAM_DEPTH 16

namespace eprosima {
namespace micrortps {

/**************************************************************************************************
 * Best-Effort Input Streams.
 **************************************************************************************************/
class BestEffortInputStream
{
public:
    BestEffortInputStream() : last_received_(~0) {}

    bool next_message(SeqNum seq_num);

private:
    SeqNum last_received_;
};

inline bool BestEffortInputStream::next_message(SeqNum seq_num)
{
    if (seq_num > last_received_)
    {
        last_received_ = seq_num;
        return true;
    }
    return false;
}

/**************************************************************************************************
 * Reliable Input Stream.
 **************************************************************************************************/
class ReliableInputStream
{
public:
    ReliableInputStream() : last_handled_(~0), last_announced_(~0) {}

    ReliableInputStream(const ReliableInputStream&) = delete;
    ReliableInputStream& operator=(const ReliableInputStream) = delete;
    ReliableInputStream(ReliableInputStream&&);
    ReliableInputStream& operator=(ReliableInputStream&&);

    bool next_message(SeqNum seq_num, InputMessagePtr& message);
    bool pop_message(InputMessagePtr& message);
    void update_from_heartbeat(SeqNum first_available, SeqNum last_available);
    SeqNum get_first_unacked() const;
    std::array<uint8_t, 2> get_nack_bitmap();

private:
    SeqNum last_handled_;
    SeqNum last_announced_;
    std::map<uint16_t, InputMessagePtr> messages_;
};

inline ReliableInputStream::ReliableInputStream(ReliableInputStream&& x)
    : last_handled_(x.last_handled_),
      last_announced_(x.last_announced_),
      messages_(std::move(x.messages_))
{
}

inline ReliableInputStream& ReliableInputStream::operator=(ReliableInputStream&& x)
{
    last_handled_ = x.last_handled_;
    last_announced_ = x.last_announced_;
    messages_ = std::move(x.messages_);
    return *this;
}

inline bool ReliableInputStream::next_message(SeqNum seq_num, InputMessagePtr& message)
{
    bool rv = false;
    if (seq_num == last_handled_ + 1)
    {
        last_handled_ += 1;
        rv = true;
    }
    else
    {
        if ((seq_num > last_handled_ + 1) && (seq_num <= last_handled_ + MICRORTPS_RELIABLE_STREAM_DEPTH))
        {
            if (seq_num > last_announced_)
            {
                last_announced_ = seq_num;
                messages_.insert(std::make_pair(seq_num, std::move(message)));
            }
            else
            {
                auto it = messages_.find(seq_num);
                if (it == messages_.end())
                {
                    messages_.insert(std::make_pair(seq_num, std::move(message)));

                }
            }
        }
    }
    return rv;
}

inline bool ReliableInputStream::pop_message(InputMessagePtr& message)
{
    bool rv = false;
    auto it = messages_.find(last_handled_ + 1);
    if (it != messages_.end())
    {
        last_handled_ += 1;
        message = std::move(messages_.at(last_handled_));
        messages_.erase(last_handled_);
        rv = true;
    }
    return rv;
}

inline void ReliableInputStream::update_from_heartbeat(SeqNum first_available, SeqNum last_available)
{
    if (last_handled_ + 1 < first_available)
    {
        last_handled_ = first_available;
    }
    if (last_announced_ < last_available)
    {
        last_announced_ = last_available;
    }
}

inline SeqNum ReliableInputStream::get_first_unacked() const
{
    return last_handled_ + 1;
}

inline std::array<uint8_t, 2> ReliableInputStream::get_nack_bitmap()
{
    std::array<uint8_t, 2> bitmap = {0, 0};
    for (uint16_t i = 0; i < 8; i++)
    {
        if (last_handled_ + i < last_announced_)
        {
            auto it = messages_.find(last_handled_ + i + 1);
            if (it == messages_.end())
            {
                bitmap.at(1) = bitmap.at(1) | (0x01 << i);
            }
        }
        if (last_handled_ + i + 8 < last_announced_)
        {
            auto it = messages_.find(last_handled_ + i + 9);
            if (it == messages_.end())
            {
                bitmap.at(0) = bitmap.at(0) | (0x01 << i);
            }
        }
    }
    return bitmap;
}

} // namespace micrortps
} // namespace eprosima

#endif //_MICRORTPS_AGENT_CLIENT_SESSION_STREAM_INPUT_STREAM_HPP_
