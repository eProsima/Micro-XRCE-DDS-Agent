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

#ifndef UXR_AGENT_CLIENT_SESSION_SESSION_HPP_
#define UXR_AGENT_CLIENT_SESSION_SESSION_HPP_

#include <uxr/agent/client/session/SessionInfo.hpp>
#include <uxr/agent/client/session/stream/InputStream.hpp>
#include <uxr/agent/client/session/stream/OutputStream.hpp>

#include <unordered_map>
#include <memory>

namespace eprosima {
namespace uxr {

inline bool is_none_stream(dds::xrce::StreamId stream_id)
{
    return (dds::xrce::STREAMID_NONE == stream_id);
}

inline bool is_besteffort_stream(dds::xrce::StreamId stream_id)
{
    return (dds::xrce::STREAMID_BUILTIN_RELIABLE > stream_id) && (dds::xrce::STREAMID_NONE != stream_id);
}

inline bool is_reliable_stream(dds::xrce::StreamId stream_id)
{
    return (dds::xrce::STREAMID_BUILTIN_RELIABLE <= stream_id);
}

class Session
{
    friend class NoneOutputStream;
    friend class BestEffortOutputStream;
    friend class ReliableOutputStream;
public:
    Session(const SessionInfo& info)
        : session_info_{info}
        , none_out_stream_{}
    {}

    ~Session() = default;

    Session(Session&& x) = delete;
    Session(const Session&) = delete;
    Session& operator=(Session&& x) = delete;
    Session& operator=(const Session&) = delete;

    void reset();

    /* Input streams functions. */
    bool is_next_input_message(InputMessagePtr& message);
    bool pop_input_message(dds::xrce::StreamId stream_id, InputMessagePtr& message);
    void update_from_heartbeat(dds::xrce::StreamId stream_id, SeqNum first_unacked, SeqNum last_unacked);
    SeqNum get_first_unacked_seq_num(dds::xrce::StreamId stream_id);
    std::array<uint8_t, 2> get_nack_bitmap(dds::xrce::StreamId stream_id);
    void push_input_fragment(dds::xrce::StreamId stream_id, InputMessagePtr& message);
    bool pop_input_fragment_message(dds::xrce::StreamId stream_id, InputMessagePtr& message);

    /* Output streams functions. */
    std::vector<uint8_t> get_output_streams();

    template<class T>
    void push_output_submessage(
            dds::xrce::StreamId stream_id,
            dds::xrce::SubmessageId submessage_id,
            const T& submessage);

    bool get_next_output_message(
            dds::xrce::StreamId stream_id,
            OutputMessagePtr& output_message);

    bool get_output_message(
            dds::xrce::StreamId stream_id,
            SeqNum seq_num,
            OutputMessagePtr& output_submessage);

    void update_from_acknack(
            dds::xrce::StreamId stream_id,
            SeqNum first_unacked);

    bool fill_heartbeat(
            dds::xrce::StreamId stream_id,
            dds::xrce::HEARTBEAT_Payload& heartbeat);

private:
    const SessionInfo session_info_;

    NoneInputStream none_in_stream_;
    std::unordered_map<dds::xrce::StreamId, BestEffortInputStream> besteffort_in_streams_;
    std::unordered_map<dds::xrce::StreamId, ReliableInputStream> reliable_in_streams_;
    std::mutex input_mtx_;

    NoneOutputStream none_out_stream_;
    std::unordered_map<dds::xrce::StreamId, BestEffortOutputStream> besteffort_out_streams_;
    std::unordered_map<dds::xrce::StreamId, ReliableOutputStream> reliable_out_streams_;
    std::mutex output_mtx_;
};

inline void Session::reset()
{
    std::unique_lock<std::mutex> input_lock(output_mtx_);
    for (auto& it : besteffort_in_streams_)
    {
        it.second.reset();
    }
    for (auto& it : reliable_in_streams_)
    {
        it.second.reset();
    }
    input_lock.unlock();

    std::lock_guard<std::mutex> output_lock(output_mtx_);
    none_out_stream_.reset();
    for (auto& it : besteffort_out_streams_)
    {
        it.second.reset();
    }
    for (auto& it : reliable_out_streams_)
    {
        it.second.reset();
    }
}

/**************************************************************************************************
 * Input Stream Methods.
 **************************************************************************************************/
inline bool Session::is_next_input_message(InputMessagePtr& message)
{
    bool rv;
    dds::xrce::StreamId stream_id = message->get_header().stream_id();
    SeqNum seq_num = message->get_header().sequence_nr();
    if (is_none_stream(stream_id))
    {
        rv = none_in_stream_.next_message(seq_num);
    }
    else if (is_besteffort_stream(stream_id))
    {
        rv = besteffort_in_streams_[stream_id].next_message(seq_num);
    }
    else
    {
        rv = reliable_in_streams_[stream_id].next_message(seq_num, message);
    }
    return rv;
}

inline bool Session::pop_input_message(dds::xrce::StreamId stream_id, InputMessagePtr& message)
{
    bool rv = false;
    if (is_reliable_stream(stream_id))
    {
        rv = reliable_in_streams_[stream_id].pop_message(message);
    }
    return rv;
}

inline void Session::update_from_heartbeat(dds::xrce::StreamId stream_id, SeqNum first_unacked, SeqNum last_unacked)
{
    if (is_reliable_stream(stream_id))
    {
        reliable_in_streams_[stream_id].update_from_heartbeat(first_unacked, last_unacked);
    }
}

inline SeqNum Session::get_first_unacked_seq_num(dds::xrce::StreamId stream_id)
{
    return reliable_in_streams_[stream_id].get_first_unacked();
}

inline std::array<uint8_t, 2> Session::get_nack_bitmap(const dds::xrce::StreamId stream_id)
{
    std::array<uint8_t, 2> bitmap = {0, 0};
    if (is_reliable_stream(stream_id))
    {
        bitmap = reliable_in_streams_[stream_id].get_nack_bitmap();
    }
    return bitmap;
}

inline void Session::push_input_fragment(dds::xrce::StreamId stream_id, InputMessagePtr& message)
{
    if (is_reliable_stream(stream_id))
    {
        reliable_in_streams_[stream_id].push_fragment(message);
    }
}

inline bool Session::pop_input_fragment_message(dds::xrce::StreamId stream_id, InputMessagePtr& message)
{
    return reliable_in_streams_[stream_id].pop_fragment_message(message);
}

/**************************************************************************************************
 * Output Stream Methods.
 **************************************************************************************************/

inline std::vector<uint8_t> Session::get_output_streams()
{
    std::vector<uint8_t> result;
    result.reserve(reliable_out_streams_.size());
    for (auto it = reliable_out_streams_.begin(); it != reliable_out_streams_.end(); ++it)
    {
        result.push_back(it->first);
    }
    return result;
}

template<class T>
inline void Session::push_output_submessage(
        dds::xrce::StreamId stream_id,
        dds::xrce::SubmessageId submessage_id,
        const T& submessage)
{
    if (is_none_stream(stream_id))
    {
        none_out_stream_.push_submessage(session_info_, submessage_id, submessage);
    }
    else if (is_besteffort_stream(stream_id))
    {
        besteffort_out_streams_[stream_id].push_submessage(session_info_, stream_id, submessage_id, submessage);
    }
    else
    {
        reliable_out_streams_[stream_id].push_submessage(session_info_, stream_id, submessage_id, submessage);
    }
}

inline bool Session::get_next_output_message(
        dds::xrce::StreamId stream_id,
        OutputMessagePtr& output_message)
{
    bool rv = false;
    if (is_none_stream(stream_id))
    {
        rv = none_out_stream_.pop_message(output_message);
    }
    else if (is_besteffort_stream(stream_id))
    {
        rv = besteffort_out_streams_[stream_id].pop_message(output_message);
    }
    else
    {
        rv = reliable_out_streams_[stream_id].get_next_message(output_message);
    }
    return rv;
}

inline bool Session::get_output_message(
        dds::xrce::StreamId stream_id,
        SeqNum seq_num,
        OutputMessagePtr& output_message)
{
    bool rv = false;
    if (is_reliable_stream(stream_id))
    {
        rv = reliable_out_streams_[stream_id].get_message(seq_num, output_message);
    }
    return rv;
}

inline void Session::update_from_acknack(
        const dds::xrce::StreamId stream_id,
        const SeqNum first_unacked)
{
    if (is_reliable_stream(stream_id))
    {
        reliable_out_streams_.at(stream_id).update_from_acknack(first_unacked);
    }
}


inline bool Session::fill_heartbeat(
        dds::xrce::StreamId stream_id,
        dds::xrce::HEARTBEAT_Payload& heartbeat)
{
    bool rv = false;
    if (is_reliable_stream(stream_id))
    {
        rv = reliable_out_streams_[stream_id].fill_heartbeat(heartbeat);
        heartbeat.stream_id(stream_id);
    }
    return rv;
}


} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_CLIENT_SESSION_SESSION_HPP_
