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

#include <micrortps/agent/client/XRCEStreams.hpp>

namespace eprosima {
namespace micrortps {

void BestEffortStream::update(SeqNum seq_num)
{
    if (seq_num_is_greater(seq_num, last_handled_))
    {
        last_handled_ = seq_num;
    }
}

ReliableInputStream::ReliableInputStream(ReliableInputStream&& x)
    : last_handled_(x.last_handled_),
      last_announced_(x.last_announced_),
      messages_(std::move(x.messages_))
{
}

ReliableInputStream& ReliableInputStream::operator=(ReliableInputStream&& x)
{
    last_handled_ = x.last_handled_;
    last_announced_ = x.last_announced_;
    messages_ = std::move(x.messages_);
    return *this;
}

void ReliableInputStream::insert_message(SeqNum index, const char* buf, size_t len)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if ((seq_num_is_greater(index, last_handled_) && seq_num_is_less(index, last_handled_ + STREAM_HISTORY_DEPTH))
            || index == last_handled_ + STREAM_HISTORY_DEPTH)
    {
        /* Update last_announced. */
        if (seq_num_is_greater(index, last_announced_))
        {
            last_announced_ = index;
        }

        /* Insert element. */
        auto it = messages_.find(index);
        if (it == messages_.end())
        {
            messages_.emplace(std::piecewise_construct,
                              std::forward_as_tuple(index),
                              std::forward_as_tuple(buf, buf + len));
        }
    }
}

bool ReliableInputStream::message_available()
{
    auto it = messages_.find(last_handled_ + 1);
    return (it != messages_.end());
}

XrceMessage ReliableInputStream::get_next_message()
{
    std::lock_guard<std::mutex> lock(mtx_);
    XrceMessage message = {nullptr, 0};
    SeqNum index = last_handled_ + 1;
    auto it = messages_.find(index);
    if (it != messages_.end())
    {
        message.buf = reinterpret_cast<char*>(messages_.at(index).data());
        message.len = messages_.at(index).size();
    }
    return message;
}

void ReliableInputStream::update_from_heartbeat(SeqNum first_available, SeqNum last_available)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (last_handled_ + 1 < first_available)
    {
        last_handled_ = first_available;
    }
    if (last_announced_ < last_available)
    {
        last_announced_ = last_available;
    }
}

void ReliableInputStream::update_from_message(SeqNum seq_num)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (seq_num == last_handled_ + 1)
    {
        last_handled_ = seq_num;
        messages_.erase(last_handled_);
    }
    if (last_announced_ < seq_num)
    {
        last_announced_ = seq_num;
    }
}

SeqNum ReliableInputStream::get_first_unacked() const
{
    return last_handled_ + 1;
}

std::array<uint8_t, 2> ReliableInputStream::get_nack_bitmap()
{
    std::lock_guard<std::mutex> lock(mtx_);
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

ReliableOutputStream::ReliableOutputStream(ReliableOutputStream&& x)
    : last_sent_(x.last_sent_),
      last_acknown_(x.last_acknown_),
      messages_(std::move(messages_))
{
}

ReliableOutputStream& ReliableOutputStream::operator=(ReliableOutputStream&& x)
{
    last_sent_ = x.last_sent_;
    last_acknown_ = x.last_acknown_;
    messages_ = std::move(x.messages_);
    return *this;
}

void ReliableOutputStream::push_message(const char* buf, size_t len)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (last_sent_ < last_acknown_ + 16)
    {
        last_sent_ += 1;
        messages_.emplace(std::piecewise_construct,
                          std::forward_as_tuple(last_sent_),
                          std::forward_as_tuple(buf, buf + len));
    }
}

XrceMessage ReliableOutputStream::get_message(SeqNum index)
{
    std::lock_guard<std::mutex> lock(mtx_);
    XrceMessage message = {nullptr, 0};
    auto it = messages_.find(index);
    if (it != messages_.end())
    {
        message.buf = reinterpret_cast<char*>(messages_.at(index).data());
        message.len = messages_.at(index).size();
    }
    return message;
}

void ReliableOutputStream::update_from_acknack(SeqNum first_unacked)
{
    std::lock_guard<std::mutex> lock(mtx_);
    while ((last_acknown_ + 1 < first_unacked) && (last_acknown_ < last_sent_))
    {
        last_acknown_ += 1;
        messages_.erase(last_acknown_);
    }
}

} // namespace micrortps
} // namespace eprosima
