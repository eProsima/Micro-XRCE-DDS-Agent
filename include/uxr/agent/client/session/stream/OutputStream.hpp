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

#ifndef UXR_AGENT_CLIENT_SESSION_STREAM_OUTPUT_STREAM_HPP_
#define UXR_AGENT_CLIENT_SESSION_STREAM_OUTPUT_STREAM_HPP_

#include <uxr/agent/config.hpp>
#include <uxr/agent/message/Packet.hpp>
#include <uxr/agent/utils/SeqNum.hpp>
#include <uxr/agent/client/session/SessionInfo.hpp>
#include <uxr/agent/logger/Logger.hpp>

#include <memory>
#include <queue>
#include <mutex>
#include <array>
#include <map>
#include <condition_variable>

namespace eprosima {
namespace uxr {

/****************************************************************************************
 * None Output Stream.
 ****************************************************************************************/
class NoneOutputStream
{
public:
    NoneOutputStream() = default;

    ~NoneOutputStream() = default;

    NoneOutputStream(NoneOutputStream&&) = delete;
    NoneOutputStream(const NoneOutputStream&) = delete;
    NoneOutputStream& operator=(NoneOutputStream&&) = delete;
    NoneOutputStream& operator=(const NoneOutputStream) = delete;

    void reset();

    template<class T>
    bool push_submessage(
            const SessionInfo& session_info,
            dds::xrce::SubmessageId id,
            const T& submessage);

    bool pop_message(OutputMessagePtr& output_message);

private:
    std::queue<OutputMessagePtr> messages_;
    std::mutex mtx_;
};

inline void NoneOutputStream::reset()
{
    std::lock_guard<std::mutex> lock(mtx_);
    while (!messages_.empty())
    {
        messages_.pop();
    }
}

template<class T>
inline bool NoneOutputStream::push_submessage(
        const SessionInfo& session_info,
        dds::xrce::SubmessageId id,
        const T& submessage)
{
    bool rv = false;
    std::lock_guard<std::mutex> lock(mtx_);
    if (BEST_EFFORT_STREAM_DEPTH > messages_.size())
    {
        /* Message header. */
        dds::xrce::MessageHeader message_header;
        message_header.session_id(session_info.session_id);
        message_header.stream_id(dds::xrce::STREAMID_NONE);
        message_header.sequence_nr(0x00);
        message_header.client_key(session_info.client_key);

        /* Create message. */
        OutputMessagePtr output_message(new OutputMessage(message_header, session_info.mtu));
        if (output_message->append_submessage(id, submessage))
        {
            /* Push message. */
            messages_.push(std::move(output_message));
            rv = true;
        }
    }
    return rv;
}

inline bool NoneOutputStream::pop_message(OutputMessagePtr& output_message)
{
    bool rv = false;
    std::lock_guard<std::mutex> lock(mtx_);
    if (!messages_.empty())
    {
        output_message = std::move(messages_.front());
        messages_.pop();
        rv = true;
    }
    return rv;
}

/****************************************************************************************
 * Best-Effort Output Stream.
 ****************************************************************************************/
class BestEffortOutputStream
{
public:
    BestEffortOutputStream()
        : last_sent_(UINT16_MAX)
    {}

    ~BestEffortOutputStream() = default;

    BestEffortOutputStream(BestEffortOutputStream&&) = delete;
    BestEffortOutputStream(const BestEffortOutputStream&) = delete;
    BestEffortOutputStream& operator=(BestEffortOutputStream&&) = delete;
    BestEffortOutputStream& operator=(const BestEffortOutputStream) = delete;

//    SeqNum get_last_handled() const { return last_sent_; }
//    void promote_stream() { last_sent_ += 1; }
    void reset();

    template<class T>
    bool push_submessage(
            const SessionInfo& session_info,
            dds::xrce::StreamId stream_id,
            dds::xrce::SubmessageId submessage_id,
            const T& submessage);

    bool pop_message(OutputMessagePtr& output_message);

private:
    std::queue<OutputMessagePtr> messages_;
    SeqNum last_sent_;
    std::mutex mtx_;
};

inline void BestEffortOutputStream::reset()
{
    std::lock_guard<std::mutex> lock(mtx_);
    while (!messages_.empty())
    {
        messages_.pop();
    }
    last_sent_ = UINT16_MAX;
}

template<class T>
inline bool BestEffortOutputStream::push_submessage(
        const SessionInfo& session_info,
        dds::xrce::StreamId stream_id,
        dds::xrce::SubmessageId submessage_id,
        const T& submessage)
{
    bool rv = false;
    std::lock_guard<std::mutex> lock(mtx_);
    if (BEST_EFFORT_STREAM_DEPTH > messages_.size())
    {
        /* Message header. */
        dds::xrce::MessageHeader message_header;
        message_header.session_id(session_info.session_id);
        message_header.stream_id(stream_id);
        message_header.sequence_nr(last_sent_ + 1);
        message_header.client_key(session_info.client_key);

        /* Create message. */
        OutputMessagePtr output_message(new OutputMessage(message_header, session_info.mtu));
        if (session_info.mtu < submessage.getCdrSerializedSize())
        {
            UXR_AGENT_LOG_WARN(
                UXR_DECORATE_YELLOW("serialization warning"),
                "Trying to serialize {:d} in {:d} MTU stream",
                submessage.getCdrSerializedSize(),
                session_info.mtu);
            rv = true;
        }
        else if (output_message->append_submessage(submessage_id, submessage))
        {
            /* Push message. */
            messages_.push(std::move(output_message));
            last_sent_ += 1;
            rv = true;
        }
    }
    return rv;
}

inline bool BestEffortOutputStream::pop_message(OutputMessagePtr& output_message)
{
    std::lock_guard<std::mutex> lock(mtx_);
    bool rv = false;
    if (!messages_.empty())
    {
        output_message = std::move(messages_.front());
        messages_.pop();
        rv = true;
    }
    return rv;
}

/****************************************************************************************
 * Reliable Output Stream.
 ****************************************************************************************/
class ReliableOutputStream
{
public:
    ReliableOutputStream()
        : last_unacked_(UINT16_MAX)
        , last_sent_(UINT16_MAX)
        , first_unacked_(0x0000)
    {}

//    bool push_message(OutputMessagePtr& output_message);

    void reset();

    template<class T>
    bool push_submessage(
            const SessionInfo& session_info,
            dds::xrce::StreamId stream_id,
            dds::xrce::SubmessageId submessage_id,
            const T& submessage,
            std::chrono::milliseconds timeout);

    bool get_next_message(OutputMessagePtr& output_message);

    bool get_message(
            SeqNum seq_num,
            OutputMessagePtr& output_message);

    void update_from_acknack(SeqNum first_unacked);

    bool fill_heartbeat(dds::xrce::HEARTBEAT_Payload& heartbeat);

private:
    std::map<uint16_t, OutputMessagePtr> messages_;
    SeqNum last_unacked_;
    SeqNum last_sent_;
    SeqNum first_unacked_;
    std::mutex mtx_;
    std::condition_variable cv_;
};

//inline bool ReliableOutputStream::push_message(OutputMessagePtr& output_message)
//{
//    bool rv = false;
//    std::lock_guard<std::mutex> lock(mtx_);
//    if (last_sent_ < last_acknown_ + SeqNum(RELIABLE_STREAM_DEPTH))
//    {
//        last_sent_ += 1;
//        messages_.insert(std::make_pair(last_sent_, output_message));
//        rv = true;
//    }
//    return rv;
//}
//
inline void ReliableOutputStream::reset()
{
    std::lock_guard<std::mutex> lock(mtx_);
    last_unacked_ = UINT16_MAX;
    last_sent_ = UINT16_MAX;
    first_unacked_ = 0x0000;
    messages_.clear();
}

template<class T>
inline bool ReliableOutputStream::push_submessage(
        const SessionInfo& session_info,
        dds::xrce::StreamId stream_id,
        dds::xrce::SubmessageId submessage_id,
        const T& submessage,
        std::chrono::milliseconds timeout)
{
    bool rv = false;
    std::unique_lock<std::mutex> lock(mtx_);
    auto now = std::chrono::steady_clock::now();

    if (cv_.wait_until(
            lock,
            now + timeout, [&](){ return last_unacked_ < first_unacked_ + SeqNum(RELIABLE_STREAM_DEPTH - 1); }))
    {
        /* Message header. */
        dds::xrce::MessageHeader message_header;
        message_header.session_id(session_info.session_id);
        message_header.stream_id(stream_id);
        message_header.client_key(session_info.client_key);

        /* Submessage header. */
        dds::xrce::SubmessageHeader submessage_header;
        submessage_header.submessage_id(submessage_id);
        submessage_header.flags(dds::xrce::FLAG_LITTLE_ENDIANNESS);
        submessage_header.submessage_length(uint16_t(submessage.getCdrSerializedSize()));

        /* Compute message size. */
        const size_t header_size = message_header.getCdrSerializedSize();
        const size_t subheader_size = submessage_header.getCdrSerializedSize();
        const size_t submessage_size = subheader_size + submessage.getCdrSerializedSize();

        /* Push submessage. */
        if ((header_size + submessage_size) <= session_info.mtu)
        {
            /* Create message. */
            last_unacked_ += 1;
            message_header.sequence_nr(last_unacked_);
            OutputMessagePtr output_message(new OutputMessage(message_header, header_size + submessage_size));
            if (output_message->append_submessage(submessage_id, submessage))
            {
                /* Push message. */
                messages_.insert(std::make_pair(last_unacked_, std::move(output_message)));
                rv = true;
            }
        }
        else
        {
            /* Serialize submessage. */
            std::unique_ptr<uint8_t[]> buf(new uint8_t[submessage_size]);
            fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(buf.get()), submessage_size);
            fastcdr::Cdr serializer(fastbuffer);
            submessage_header.serialize(serializer);
            submessage.serialize(serializer);

            const size_t max_fragment_size = session_info.mtu - header_size - subheader_size;
            dds::xrce::SubmessageHeader fragment_subheader;
            fragment_subheader.submessage_id(dds::xrce::FRAGMENT);
            fragment_subheader.flags(dds::xrce::FLAG_LITTLE_ENDIANNESS);
            fragment_subheader.submessage_length(uint16_t(max_fragment_size));

            uint16_t serialized_size = 0;
            do
            {
                uint16_t fragment_size;
                if (session_info.mtu < (header_size + subheader_size + (submessage_size - serialized_size)))
                {
                    fragment_size = uint16_t(max_fragment_size);
                }
                else
                {
                    fragment_size = uint16_t(submessage_size - serialized_size);
                    fragment_subheader.flags(submessage_header.flags() | dds::xrce::FLAG_LAST_FRAGMENT);
                }
                fragment_subheader.submessage_length(fragment_size);

                const size_t current_message_size = header_size + subheader_size + fragment_size;

                /* Create message. */
                last_unacked_ += 1;
                message_header.sequence_nr(last_unacked_);
                OutputMessagePtr output_message(new OutputMessage(message_header, current_message_size));
                if (output_message->append_fragment(fragment_subheader,  buf.get() + serialized_size, fragment_size))
                {
                    /* Push message. */
                    messages_.insert(std::make_pair(last_unacked_, std::move(output_message)));
                    serialized_size += fragment_size;
                }
                else
                {
                    break;
                }

            } while (serialized_size < submessage_size);
            rv = (serialized_size == submessage_size);
        }
    }
    return rv;
}

inline bool ReliableOutputStream::get_next_message(OutputMessagePtr& output_message)
{
    bool rv = false;
    std::lock_guard<std::mutex> lock(mtx_);
    if (last_sent_ < last_unacked_)
    {
        last_sent_ += 1;
        output_message = messages_.at(last_sent_);
        rv = true;
    }
    return rv;
}

inline bool ReliableOutputStream::get_message(
        SeqNum seq_num,
        OutputMessagePtr& output_message)
{
    bool rv = false;
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = messages_.find(seq_num);
    if (it != messages_.end())
    {
        output_message = it->second;
        rv = true;
    }
    return rv;
}

inline void ReliableOutputStream::update_from_acknack(SeqNum first_unacked)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (first_unacked <= last_sent_ + 1)
    {
        while (first_unacked > first_unacked_)
        {
            messages_.erase(first_unacked_);
            first_unacked_ += 1;
        }
        cv_.notify_one();
    }
}

inline bool ReliableOutputStream::fill_heartbeat(dds::xrce::HEARTBEAT_Payload& heartbeat)
{
    std::lock_guard<std::mutex> lock(mtx_);
    heartbeat.first_unacked_seq_nr(first_unacked_);
    heartbeat.last_unacked_seq_nr(last_unacked_);
    return !messages_.empty();
}

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_CLIENT_SESSION_STREAM_OUTPUT_STREAM_HPP_
