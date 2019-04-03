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

#ifndef _UXR_AGENT_CLIENT_SESSION_STREAM_OUTPUT_STREAM_HPP_
#define _UXR_AGENT_CLIENT_SESSION_STREAM_OUTPUT_STREAM_HPP_

#include <uxr/agent/config.hpp>
#include <uxr/agent/message/Packet.hpp>
#include <uxr/agent/utils/SeqNum.hpp>

#include <memory>
#include <queue>
#include <mutex>
#include <array>
#include <map>

namespace eprosima {
namespace uxr {

class Session;

/****************************************************************************************
 * Output Stream.
 ****************************************************************************************/
class OutputStream
{
public:
    OutputStream(
            const Session& session,
            dds::xrce::StreamId stream_id)
        : session_{std::move(session)}
        , stream_id_{stream_id}
        , mtx_{}
    {}

protected:
    const Session& session_;
    const dds::xrce::StreamId stream_id_;
    std::mutex mtx_;
};

/****************************************************************************************
 * None Output Stream.
 ****************************************************************************************/
class NoneOutputStream : public OutputStream
{
public:
    NoneOutputStream(
            const Session& session,
            const dds::xrce::ClientKey& client_key,
            size_t mtu)
        : OutputStream(session, dds::xrce::STREAMID_NONE)
        , stream_id_(dds::xrce::STREAMID_NONE)
        , client_key_(client_key)
        , mtu_(mtu)
    {}

    ~NoneOutputStream() = default;

    NoneOutputStream(NoneOutputStream&&) = delete;
    NoneOutputStream(const NoneOutputStream&) = delete;
    NoneOutputStream& operator=(NoneOutputStream&&) = delete;
    NoneOutputStream& operator=(const NoneOutputStream) = delete;

    template<class T>
    bool push_submessage(dds::xrce::SubmessageId id, const T& submessage);
    bool get_next_message(OutputMessagePtr& output_message);

private:
    const dds::xrce::StreamId stream_id_;
    const dds::xrce::ClientKey client_key_;
    const size_t mtu_;
    std::queue<OutputMessagePtr> messages_;
    std::mutex mtx_;
};

template<class T>
inline bool NoneOutputStream::push_submessage(dds::xrce::SubmessageId id, const T& submessage)
{
    bool rv = false;
    std::lock_guard<std::mutex> lock(mtx_);
    if (BEST_EFFORT_STREAM_DEPTH > messages_.size())
    {
        /* Message header. */
        dds::xrce::MessageHeader message_header;
        message_header.session_id(session_.id_);
        message_header.stream_id(stream_id_);
        message_header.sequence_nr(0x00);
        message_header.client_key(client_key_);

        /* Create message. */
        OutputMessagePtr output_message(new OutputMessage(message_header, mtu_));
        if (output_message->append_submessage(id, submessage))
        {
            /* Push message. */
            messages_.push(std::move(output_message));
            rv = true;
        }
    }
    return rv;
}

inline bool NoneOutputStream::get_next_message(OutputMessagePtr& output_message)
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
    BestEffortOutputStream(dds::xrce::SessionId session_id,
                           dds::xrce::StreamId stream_id,
                           const dds::xrce::ClientKey& client_key,
                           size_t mtu)
        : session_id_(session_id),
          stream_id_(stream_id),
          client_key_(client_key),
          mtu_(mtu),
          last_sent_(UINT16_MAX)
    {}

    ~BestEffortOutputStream() = default;

    BestEffortOutputStream(BestEffortOutputStream&&) = delete;
    BestEffortOutputStream(const BestEffortOutputStream&) = delete;
    BestEffortOutputStream& operator=(BestEffortOutputStream&&) = delete;
    BestEffortOutputStream& operator=(const BestEffortOutputStream) = delete;

    SeqNum get_last_handled() const { return last_sent_; }
    void promote_stream() { last_sent_ += 1; }
    void reset() { last_sent_ = UINT16_MAX; }

    template<class T>
    bool push_submessage(dds::xrce::SubmessageId id, const T& submessage);
    bool get_next_message(OutputMessagePtr& output_message);

private:
    const dds::xrce::SessionId session_id_;
    const dds::xrce::StreamId stream_id_;
    const dds::xrce::ClientKey client_key_;
    const size_t mtu_;
    SeqNum last_sent_;
    std::queue<OutputMessagePtr> messages_;
    std::mutex mtx_;
};

template<class T>
inline bool BestEffortOutputStream::push_submessage(dds::xrce::SubmessageId id, const T& submessage)
{
    bool rv = false;
    std::lock_guard<std::mutex> lock(mtx_);
    if (BEST_EFFORT_STREAM_DEPTH > messages_.size())
    {
        /* Message header. */
        dds::xrce::MessageHeader message_header;
        message_header.session_id(session_id_);
        message_header.stream_id(stream_id_);
        message_header.sequence_nr(last_sent_ + 1);
        message_header.client_key(client_key_);

        /* Create message. */
        OutputMessagePtr output_message(new OutputMessage(message_header, mtu_));
        if (output_message->append_submessage(id, submessage))
        {
            /* Push message. */
            messages_.push(std::move(output_message));
            last_sent_ += 1;
            rv = true;
        }
    }
    return rv;
}

inline bool BestEffortOutputStream::get_next_message(OutputMessagePtr& output_message)
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
    ReliableOutputStream(dds::xrce::SessionId session_id,
                         dds::xrce::StreamId stream_id,
                         const dds::xrce::ClientKey& client_key,
                         size_t mtu)
        : session_id_(session_id),
          stream_id_(stream_id),
          client_key_(client_key),
          mtu_(mtu),
          last_available_(UINT16_MAX),
          last_sent_(UINT16_MAX),
          last_acknown_(UINT16_MAX)
    {}

    ReliableOutputStream(ReliableOutputStream&&) = delete;
    ReliableOutputStream(const ReliableOutputStream&) = delete;
    ReliableOutputStream& operator=(ReliableOutputStream&&) = delete;
    ReliableOutputStream& operator=(const ReliableOutputStream) = delete;

    bool push_message(OutputMessagePtr& output_message);
    bool get_message(SeqNum seq_num, OutputMessagePtr& output_message);
    void update_from_acknack(SeqNum first_unacked);
    SeqNum get_first_available() { return last_acknown_ + 1; }
    SeqNum get_last_available() { return last_available_; }
    SeqNum next_message() { return last_sent_ + 1; }
    bool message_pending() { return !messages_.empty(); }
    void reset();

    /* Fragment related functions. */
    template<class T>
    bool push_submessage(dds::xrce::SubmessageId id, const T& submessage);
    bool get_next_message(OutputMessagePtr& output_message);

private:
    const dds::xrce::SessionId session_id_;
    const dds::xrce::StreamId stream_id_;
    const dds::xrce::ClientKey client_key_;
    const size_t mtu_;
    SeqNum last_available_;
    SeqNum last_sent_;
    SeqNum last_acknown_;
    std::map<uint16_t, OutputMessagePtr> messages_;
    std::mutex mtx_;
};

inline bool ReliableOutputStream::push_message(OutputMessagePtr& output_message)
{
    bool rv = false;
    std::lock_guard<std::mutex> lock(mtx_);
    if (last_sent_ < last_acknown_ + SeqNum(RELIABLE_STREAM_DEPTH))
    {
        last_sent_ += 1;
        messages_.insert(std::make_pair(last_sent_, output_message));
        rv = true;
    }
    return rv;
}

inline bool ReliableOutputStream::get_message(SeqNum seq_num, OutputMessagePtr& output_message)
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

    if ((last_acknown_ + 1 < first_unacked) && (last_sent_ + 1 >= first_unacked))
    {
        while (last_acknown_ + 1 < first_unacked)
        {
            last_acknown_ += 1;
            messages_.erase(last_acknown_);
        }
    }
}

inline void ReliableOutputStream::reset()
{
    std::lock_guard<std::mutex> lock(mtx_);
    last_available_ = UINT16_MAX;
    last_sent_ = UINT16_MAX;
    last_acknown_ = UINT16_MAX;
    messages_.clear();
}

/* Fragment related functions. */
template<class T>
inline bool ReliableOutputStream::push_submessage(dds::xrce::SubmessageId id, const T& submessage)
{
    bool rv = false;
    std::lock_guard<std::mutex> lock(mtx_);
    if (last_available_ < last_acknown_ + SeqNum(RELIABLE_STREAM_DEPTH))
    {
        /* Message header. */
        dds::xrce::MessageHeader message_header;
        message_header.session_id(session_id_);
        message_header.stream_id(stream_id_);
        message_header.client_key(client_key_);

        /* Submessage header. */
        dds::xrce::SubmessageHeader submessage_header;
        submessage_header.submessage_id(id);
        submessage_header.flags(dds::xrce::FLAG_LITTLE_ENDIANNESS);
        submessage_header.submessage_length(uint16_t(submessage.getCdrSerializedSize()));

        /* Compute message size. */
        const size_t header_size = message_header.getCdrSerializedSize();
        const size_t subheader_size = submessage_header.getCdrSerializedSize();
        const size_t submessage_size = subheader_size + submessage.getCdrSerializedSize();

        /* Push submessage. */
        if ((header_size + submessage_size) <= mtu_)
        {
            /* Create message. */
            last_available_ += 1;
            message_header.sequence_nr(last_available_);
            OutputMessagePtr output_message(new OutputMessage(message_header, header_size + submessage_size));
            if (output_message->append_submessage(id, submessage))
            {
                /* Push message. */
                messages_.insert(std::make_pair(last_available_, std::move(output_message)));
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

            const size_t max_fragment_size = mtu_ - header_size - subheader_size;
            dds::xrce::SubmessageHeader fragment_subheader;
            fragment_subheader.submessage_id(dds::xrce::FRAGMENT);
            fragment_subheader.flags(dds::xrce::FLAG_LITTLE_ENDIANNESS);
            fragment_subheader.submessage_length(uint16_t(max_fragment_size));

            uint16_t serialized_size = 0;
            do
            {
                uint16_t fragment_size;
                if (mtu_ < (header_size + subheader_size + (submessage_size - serialized_size)))
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
                last_available_ += 1;
                message_header.sequence_nr(last_available_);
                OutputMessagePtr output_message(new OutputMessage(message_header, current_message_size));
                if (output_message->append_fragment(fragment_subheader,  buf.get() + serialized_size, fragment_size))
                {
                    /* Push message. */
                    messages_.insert(std::make_pair(last_available_, std::move(output_message)));
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
    if (last_sent_ < last_available_)
    {
        last_sent_ += 1;
        output_message = messages_.at(last_sent_);
        rv = true;
    }
    return rv;
}

} // namespace uxr
} // namespace eprosima

#endif //_UXR_AGENT_CLIENT_SESSION_STREAM_OUTPUT_STREAM_HPP_
