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
#include <map>
#include <queue>

namespace eprosima {
namespace uxr {

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
          last_send_(~0)
    {}

    bool push_message(OutputMessagePtr&& output_message);
    bool pop_message(OutputMessagePtr& output_message);
    SeqNum get_last_handled() const { return last_send_; }
    void promote_stream() { last_send_ += 1; }
    void reset() { last_send_ = ~0; }

    /* Fragment related functions. */
    template<class T>
    void push_submessage(dds::xrce::SubmessageId id, const T& submessage);
    bool get_next_message(OutputMessagePtr& output_message);

private:
    dds::xrce::SessionId session_id_;
    dds::xrce::StreamId stream_id_;
    dds::xrce::ClientKey client_key_;
    size_t mtu_;
    SeqNum last_send_;
    std::queue<OutputMessagePtr> messages_;
};

inline bool BestEffortOutputStream::push_message(OutputMessagePtr&& output_message)
{
    bool rv = false;
    if (BEST_EFFORT_STREAM_DEPTH > messages_.size())
    {
        messages_.push(std::move(output_message));
        rv = true;
    }
    return rv;
}

inline bool BestEffortOutputStream::pop_message(OutputMessagePtr& output_message)
{
    bool rv = false;
    if (!messages_.empty())
    {
        output_message = std::move(messages_.front());
        messages_.pop();
        rv = true;
    }
    return rv;
}

/* Fragment related functions. */
template<class T>
inline void BestEffortOutputStream::push_submessage(dds::xrce::SubmessageId id, const T& submessage)
{
    if (BEST_EFFORT_STREAM_DEPTH > messages_.size())
    {
        last_send_ += 1;

        /* Message header. */
        dds::xrce::MessageHeader message_header;
        message_header.session_id(session_id_);
        message_header.stream_id(stream_id_);
        message_header.sequence_nr(last_send_);
        message_header.client_key(client_key_);

        /* Create message. */
        OutputMessagePtr output_message(new OutputMessage(message_header, mtu_));
        output_message->append_submessage(id, submessage);

        /* Push message. */
        messages_.push(std::move(output_message));
    }
}

inline bool BestEffortOutputStream::get_next_message(OutputMessagePtr& output_message)
{
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
          last_available_(~0),
          last_sent_(~0),
          last_acknown_(~0)
    {}

    ReliableOutputStream(const ReliableOutputStream&) = delete;
    ReliableOutputStream& operator=(const ReliableOutputStream) = delete;
    ReliableOutputStream(ReliableOutputStream&&);
    ReliableOutputStream& operator=(ReliableOutputStream&&);

    bool push_message(OutputMessagePtr& output_message);
    bool get_message(SeqNum seq_num, OutputMessagePtr& output_message);
    void update_from_acknack(SeqNum first_unacked);
    SeqNum get_first_available() { return last_acknown_ + 1; }
    SeqNum get_last_available() { return last_sent_; }
    SeqNum next_message() { return last_sent_ + 1; }
    bool message_pending() { return !messages_.empty(); }
    void reset();

    /* Fragment related functions. */
    template<class T>
    void push_submessage(dds::xrce::SubmessageId id, const T& submessage);
    bool get_next_message(OutputMessagePtr& output_message);

private:
    dds::xrce::SessionId session_id_;
    dds::xrce::StreamId stream_id_;
    dds::xrce::ClientKey client_key_;
    size_t mtu_;
    SeqNum last_available_;
    SeqNum last_sent_;
    SeqNum last_acknown_;
    std::map<uint16_t, OutputMessagePtr> messages_;
};

inline ReliableOutputStream::ReliableOutputStream(ReliableOutputStream&& x)
    : last_sent_(x.last_sent_),
      last_acknown_(x.last_acknown_),
      messages_(std::move(messages_))
{}

inline ReliableOutputStream& ReliableOutputStream::operator=(ReliableOutputStream&& x)
{
    last_sent_ = x.last_sent_;
    last_acknown_ = x.last_acknown_;
    messages_ = std::move(x.messages_);
    return *this;
}

inline bool ReliableOutputStream::push_message(OutputMessagePtr& output_message)
{
    bool rv = false;
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
    while ((last_acknown_ + 1 < first_unacked) && (last_acknown_ < last_sent_))
    {
        last_acknown_ += 1;
        messages_.erase(last_acknown_);
    }
}

inline void ReliableOutputStream::reset()
{
    last_available_ = ~0;
    last_sent_ = ~0;
    last_acknown_ = ~0;
    messages_.clear();
}

/* Fragment related functions. */
template<class T>
inline void ReliableOutputStream::push_submessage(dds::xrce::SubmessageId id, const T& submessage)
{
    if (last_available_ < last_acknown_ + SeqNum(RELIABLE_STREAM_DEPTH))
    {
        /* Message header. */
        dds::xrce::MessageHeader message_header;
        message_header.session_id(session_id_);
        message_header.stream_id(stream_id_);
        message_header.sequence_nr(last_available_);
        message_header.client_key(client_key_);

        /* Submessage header. */
        dds::xrce::SubmessageHeader submessage_header;
        submessage_header.submessage_id(id);
        submessage_header.flags(0x01);
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
            output_message->append_submessage(id, submessage);

            /* Push message. */
            messages_.insert(std::make_pair(last_available_, std::move(output_message)));
        }
        else
        {
            /* Serialize submessage. */
            std::unique_ptr<uint8_t> buf(new uint8_t[submessage_size]);
            fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(buf.get()), submessage_size);
            fastcdr::Cdr serializer(fastbuffer);
            submessage_header.serialize(serializer);
            submessage.serialize(serializer);

            const size_t max_fragment_size = mtu_ - header_size - subheader_size;
            dds::xrce::SubmessageHeader fragment_subheader;
            fragment_subheader.submessage_id(dds::xrce::FRAGMENT);
            fragment_subheader.flags(0x01);
            fragment_subheader.submessage_length(uint16_t(max_fragment_size));

            uint16_t serialized_size = 0;
            while (serialized_size < submessage_size)
            {
                uint16_t fragment_size;
                if (mtu_ < (header_size + subheader_size + (submessage_size - serialized_size)))
                {
                    fragment_size = uint16_t(max_fragment_size);
                }
                else
                {
                    fragment_size = uint16_t(submessage_size - serialized_size);
                    fragment_subheader.flags(0x01 | dds::xrce::FLAG_LAST_FRAGMENT);
                }
                fragment_subheader.submessage_length(fragment_size);

                const size_t current_message_size = header_size + subheader_size + fragment_size;

                /* Create message. */
                last_available_ += 1;
                message_header.sequence_nr(last_available_);
                OutputMessagePtr output_message(new OutputMessage(message_header, current_message_size));
                output_message->append_fragment(fragment_subheader,  buf.get() + serialized_size, fragment_size);

                /* Push message. */
                messages_.insert(std::make_pair(last_available_, std::move(output_message)));

                serialized_size += fragment_size;
            }
        }
    }
}

inline bool ReliableOutputStream::get_next_message(OutputMessagePtr& output_message)
{
    bool rv = false;
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
