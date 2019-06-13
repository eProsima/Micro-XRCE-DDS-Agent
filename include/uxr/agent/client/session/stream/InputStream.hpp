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

#ifndef _UXR_AGENT_CLIENT_SESSION_STREAM_INPUT_STREAM_HPP_
#define _UXR_AGENT_CLIENT_SESSION_STREAM_INPUT_STREAM_HPP_

#include <uxr/agent/config.hpp>
#include <uxr/agent/message/Packet.hpp>
#include <uxr/agent/utils/SeqNum.hpp>
#include <uxr/agent/client/session/SessionInfo.hpp>

#include <map>
#include <mutex>
#include <queue>

namespace eprosima {
namespace uxr {

/**************************************************************************************************
 * None Input Streams.
 **************************************************************************************************/
class NoneInputStream
{
public:
    NoneInputStream() = default;

    bool push_message(
            InputMessagePtr&& input_message);

    template<typename ... Args>
    bool emplace_message(
            Args&& ... args);

    bool pop_message(
            InputMessagePtr& input_message);

    void reset();

private:
    std::queue<InputMessagePtr> messages_;
    std::mutex mtx_;
};

inline bool NoneInputStream::push_message(
        InputMessagePtr&& input_message)
{
    bool rv = false;
    std::lock_guard<std::mutex> lock(mtx_);
    if (messages_.size() < BEST_EFFORT_STREAM_DEPTH)
    {
        messages_.push(std::move(input_message));
        rv = true;
    }
    return rv;
}

template<typename ... Args>
inline bool NoneInputStream::emplace_message(
        Args&& ... args)
{
    bool rv = false;
    std::lock_guard<std::mutex> lock(mtx_);
    if (messages_.size() < BEST_EFFORT_STREAM_DEPTH)
    {
        messages_.emplace(new InputMessage(std::forward<Args>(args)...));
        rv = true;
    }
    return rv;
}

inline bool NoneInputStream::pop_message(InputMessagePtr& input_message)
{
    bool rv = false;
    std::lock_guard<std::mutex> lock(mtx_);
    if (!messages_.empty())
    {
        input_message = std::move(messages_.front());
        messages_.pop();
        rv = true;
    }
    return rv;
}

inline void NoneInputStream::reset()
{
    std::lock_guard<std::mutex> lock(mtx_);
    while (!messages_.empty())
    {
        messages_.pop();
    }
}

/**************************************************************************************************
 * Best-Effort Input Streams.
 **************************************************************************************************/
class BestEffortInputStream
{
public:
    BestEffortInputStream()
        : last_received_(UINT16_MAX)
    {}

    ~BestEffortInputStream() = default;

    BestEffortInputStream(BestEffortInputStream&&) = delete;
    BestEffortInputStream(const BestEffortInputStream&) = delete;
    BestEffortInputStream& operator=(BestEffortInputStream&&) = delete;
    BestEffortInputStream& operator=(const BestEffortInputStream) = delete;

    bool push_message(
            SeqNum seq_num,
            InputMessagePtr&& input_message);

    template<typename ... Args>
    bool emplace_message(
            SeqNum seq_num,
            Args&& ... args);

    bool pop_message(InputMessagePtr& input_message);

    void reset();

private:
    std::queue<InputMessagePtr> messages_;
    SeqNum last_received_;
    std::mutex mtx_;
};

inline bool BestEffortInputStream::push_message(
        SeqNum seq_num,
        InputMessagePtr&& input_message)
{
    bool rv = false;
    std::lock_guard<std::mutex> lock(mtx_);
    if ((seq_num > last_received_) && (messages_.size() < BEST_EFFORT_STREAM_DEPTH))
    {
        messages_.push(std::move(input_message));
        last_received_ = seq_num;
        rv = true;
    }
    return rv;
}

template<typename ... Args>
inline bool BestEffortInputStream::emplace_message(
        SeqNum seq_num,
        Args&& ... args)
{
    bool rv = false;
    std::lock_guard<std::mutex> lock(mtx_);
    if ((seq_num > last_received_) && (messages_.size() < BEST_EFFORT_STREAM_DEPTH))
    {
        messages_.emplace(new InputMessage(std::forward<Args>(args)...));
        last_received_ = seq_num;
        rv = true;
    }
    return rv;
}

inline bool BestEffortInputStream::pop_message(InputMessagePtr& input_message)
{
    bool rv = false;
    std::lock_guard<std::mutex> lock(mtx_);
    if (!messages_.empty())
    {
        input_message = std::move(messages_.front());
        messages_.pop();
        rv = true;
    }
    return rv;
}

inline void BestEffortInputStream::reset()
{
    std::lock_guard<std::mutex> lock(mtx_);
    while (!messages_.empty())
    {
        messages_.pop();
    }
    last_received_ = UINT16_MAX;
}

/**************************************************************************************************
 * Reliable Input Stream.
 **************************************************************************************************/
class ReliableInputStream
{
public:
    ReliableInputStream()
        : last_handled_(UINT16_MAX),
          last_announced_(UINT16_MAX),
          fragment_msg_{},
          fragment_message_available_(false)
    {}

    ~ReliableInputStream() = default;

    ReliableInputStream(ReliableInputStream&&) = delete;
    ReliableInputStream(const ReliableInputStream&) = delete;
    ReliableInputStream& operator=(ReliableInputStream&&) = delete;
    ReliableInputStream& operator=(const ReliableInputStream) = delete;

    bool push_message(
            SeqNum seq_num,
            InputMessagePtr&& message);

    template<typename ... Args>
    bool emplace_message(
            SeqNum seq_num,
            Args ... args);

    bool pop_message(InputMessagePtr& message);

    void update_from_heartbeat(
            SeqNum first_unacked,
            SeqNum last_unacked);

    void fill_acknack(dds::xrce::ACKNACK_Payload& acknack);

    void push_fragment(InputMessagePtr& message);

    bool pop_fragment_message(InputMessagePtr& message);

    void reset();

private:
    SeqNum last_handled_;
    SeqNum last_announced_;
    std::map<uint16_t, InputMessagePtr> messages_;
    std::vector<uint8_t> fragment_msg_;
    bool fragment_message_available_;
    std::mutex mtx_;
};

inline bool ReliableInputStream::push_message(
        SeqNum seq_num,
        InputMessagePtr&& message)
{
    bool rv = false;
    std::lock_guard<std::mutex> lock(mtx_);
    if ((seq_num > last_handled_) && (seq_num <= last_handled_ + SeqNum(RELIABLE_STREAM_DEPTH)))
    {
        if (seq_num > last_announced_)
        {
            last_announced_ = seq_num;
            messages_.emplace(seq_num, std::move(message));
            rv = true;
        }
        else
        {
            auto it = messages_.find(seq_num);
            if (it == messages_.end())
            {
                messages_.emplace(seq_num, std::move(message));
                rv = true;
            }
        }
    }
    return rv;
}

inline bool ReliableInputStream::pop_message(InputMessagePtr& message)
{
    bool rv = false;
    std::lock_guard<std::mutex> lock(mtx_);
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

template<typename ... Args>
inline bool ReliableInputStream::emplace_message(
        SeqNum seq_num,
        Args ... args)
{
    bool rv = false;
    std::lock_guard<std::mutex> lock(mtx_);
    if ((seq_num > last_handled_) && (seq_num <= last_handled_ + SeqNum(RELIABLE_STREAM_DEPTH)))
    {
        if (seq_num > last_announced_)
        {
            last_announced_ = seq_num;
            // gcc5 doesn't not support raw pointer as argument in std::pair when std::unique_ptr is create.
            messages_.emplace(seq_num,
                              std::unique_ptr<InputMessage>(new InputMessage(std::forward<Args>(args)...)));
            rv = true;
        }
        else
        {
            auto it = messages_.find(seq_num);
            if (it == messages_.end())
            {
                messages_.emplace(seq_num,
                                  std::unique_ptr<InputMessage>(new InputMessage(std::forward<Args>(args)...)));
                rv = true;
            }
        }
    }
    return rv;
}

inline void ReliableInputStream::update_from_heartbeat(
        SeqNum first_unacked,
        SeqNum last_unacked)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (last_handled_ + 1 < first_unacked)
    {
        last_handled_ = first_unacked - 1;
    }
    if (last_announced_ < last_unacked)
    {
        last_announced_ = last_unacked;
    }
}

inline void ReliableInputStream::fill_acknack(dds::xrce::ACKNACK_Payload& acknack)
{
    acknack.nack_bitmap() = {0, 0};
    std::lock_guard<std::mutex> lock(mtx_);
    acknack.first_unacked_seq_num(last_handled_ + 1);
    for (uint16_t i = 0; i < 8; i++)
    {
        if (last_handled_ + SeqNum(i) < last_announced_)
        {
            auto it = messages_.find(last_handled_ + SeqNum(i + 1));
            if (it == messages_.end())
            {
                acknack.nack_bitmap().at(1) = acknack.nack_bitmap().at(1) | (0x01 << i);
            }
        }
        if (last_handled_ + SeqNum(i + 8) < last_announced_)
        {
            auto it = messages_.find(last_handled_ + SeqNum(i + 9));
            if (it == messages_.end())
            {
                acknack.nack_bitmap().at(0) = acknack.nack_bitmap().at(0) | (0x01 << i);
            }
        }
    }
}

inline void ReliableInputStream::reset()
{
    std::lock_guard<std::mutex> lock(mtx_);
    last_handled_ = UINT16_MAX;
    last_announced_ = UINT16_MAX;
    messages_.clear();
}

inline void ReliableInputStream::push_fragment(InputMessagePtr& message)
{
    std::lock_guard<std::mutex> lock(mtx_);

    /* Add header in case. */
    if (fragment_msg_.empty())
    {
        std::array<uint8_t, 8> raw_header;
        uint8_t header_size = message->get_raw_header(raw_header);
        fragment_msg_.insert(fragment_msg_.begin(), std::begin(raw_header), std::begin(raw_header) + header_size);
    }

    /* Append fragment. */
    size_t position = fragment_msg_.size();
    size_t fragment_size = message->get_subheader().submessage_length();
    fragment_msg_.resize(position + fragment_size);
    message->get_raw_payload(fragment_msg_.data() + position, fragment_size);

    /* Check if last message. */
    fragment_message_available_ = (0 != (dds::xrce::FLAG_LAST_FRAGMENT & message->get_subheader().flags()));
}

inline bool ReliableInputStream::pop_fragment_message(InputMessagePtr& message)
{
    bool rv = false;
    std::lock_guard<std::mutex> lock(mtx_);
    if (fragment_message_available_)
    {
        message.reset(new InputMessage(fragment_msg_.data(), fragment_msg_.size()));
        fragment_msg_.clear();
        fragment_message_available_ = false;
        return true;
    }
    return rv;
}

} // namespace uxr
} // namespace eprosima

#endif //_UXR_AGENT_CLIENT_SESSION_STREAM_INPUT_STREAM_HPP_
