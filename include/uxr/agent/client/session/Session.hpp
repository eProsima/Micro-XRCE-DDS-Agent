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

#ifndef _UXR_AGENT_CLIENT_SESSION_SESSION_HPP_
#define _UXR_AGENT_CLIENT_SESSION_SESSION_HPP_

#include <uxr/agent/client/session/stream/InputStream.hpp>
#include <uxr/agent/client/session/stream/OutputStream.hpp>
#include <unordered_map>
#include <memory>
#include <mutex>

namespace eprosima {
namespace uxr {

inline bool is_none_stream(dds::xrce::StreamId stream_id)
{
    return (0x00 == stream_id);
}

inline bool is_besteffort_stream(dds::xrce::StreamId stream_id)
{
    return (128 > stream_id) && (0x00 != stream_id);
}

inline bool is_relable_stream(dds::xrce::StreamId stream_id)
{
    return (127 < stream_id);
}

class Session
{
public:
    Session(dds::xrce::SessionId session_id, const dds::xrce::ClientKey& client_key, size_t mtu)
        : none_out_stream_(session_id, client_key, mtu)
    {
        /* Create Best-Effort output streams. */
        for (int i = 1; i <= 127; ++i)
        {
            dds::xrce::StreamId stream_id = dds::xrce::StreamId(i);
            besteffort_out_streams_.emplace(std::piecewise_construct,
                                            std::forward_as_tuple(dds::xrce::StreamId(i)),
                                            std::forward_as_tuple(session_id, stream_id, client_key, mtu));
        }

        /* Create Relaible output streams. */
        for (int i = 128; i <= 255; ++i)
        {
            dds::xrce::StreamId stream_id = dds::xrce::StreamId(i);
            reliable_out_streams_.emplace(std::piecewise_construct,
                                          std::forward_as_tuple(dds::xrce::StreamId(i)),
                                          std::forward_as_tuple(session_id, stream_id, client_key, mtu));
        }
    }

    ~Session() = default;

    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;
    Session(Session&& x) = delete;
    Session& operator=(Session&& x) = delete;

    void reset();

    /* Input streams functions. */
    bool next_input_message(InputMessagePtr& message);
    bool pop_input_message(dds::xrce::StreamId stream_id, InputMessagePtr& message);
    void update_from_heartbeat(dds::xrce::StreamId stream_id, SeqNum first_unacked, SeqNum last_unacked);
    SeqNum get_first_unacked_seq_num(dds::xrce::StreamId stream_id);
    std::array<uint8_t, 2> get_nack_bitmap(dds::xrce::StreamId stream_id);
    void push_input_fragment(dds::xrce::StreamId stream_id, InputMessagePtr& message);
    bool pop_input_fragment_message(dds::xrce::StreamId stream_id, InputMessagePtr& message);

    /* Output streams functions. */
    bool push_output_message(dds::xrce::StreamId stream_id, OutputMessagePtr& output_message);
    bool get_output_message(dds::xrce::StreamId stream_id, SeqNum seq_num, OutputMessagePtr& output_submessage);
    SeqNum get_first_unacked_seq_nr(dds::xrce::StreamId stream_id);
    SeqNum get_last_unacked_seq_nr(dds::xrce::StreamId stream_id);
    void update_from_acknack(dds::xrce::StreamId stream_id, SeqNum first_unacked);
//    SeqNum next_output_message(dds::xrce::StreamId stream_id);
    std::vector<uint8_t> get_output_streams();
    bool message_pending(dds::xrce::StreamId stream_id);

    /* Fragment relate function. */
    template<class T>
    void push_output_submessage(dds::xrce::StreamId stream_id, dds::xrce::SubmessageId, const T& submessage);
    bool get_next_output_message(dds::xrce::StreamId stream_id, OutputMessagePtr& output_message);

private:
    NoneInputStream none_in_stream_;
    std::unordered_map<dds::xrce::StreamId, BestEffortInputStream> besteffort_in_streams_;
    std::unordered_map<dds::xrce::StreamId, ReliableInputStream> reliable_in_streams_;
    NoneOutputStream none_out_stream_;
    std::unordered_map<dds::xrce::StreamId, BestEffortOutputStream> besteffort_out_streams_;
    std::unordered_map<dds::xrce::StreamId, ReliableOutputStream> reliable_out_streams_;
    std::mutex bi_mtx_;
    std::mutex ri_mtx_;
    std::mutex bo_mtx_;
    std::mutex ro_mtx_;
};

inline void Session::reset()
{
    /* Reset Best-Effor Input streams. */
    std::unique_lock<std::mutex> bi_lock(bi_mtx_);
    for (auto& it : besteffort_in_streams_)
    {
        it.second.reset();
    }
    bi_lock.unlock();

    /* Reset Reliable Input streams. */
    std::unique_lock<std::mutex> ri_lock(ri_mtx_);
    for (auto& it : reliable_in_streams_)
    {
        it.second.reset();
    }
    ri_lock.unlock();

    /* Reset Best-Effor Output streams. */
    std::unique_lock<std::mutex> bo_lock(bo_mtx_);
    for (auto& it : besteffort_out_streams_)
    {
        it.second.reset();
    }
    bo_lock.unlock();

    /* Reset Reliable Output streams. */
    std::unique_lock<std::mutex> ro_lock(ro_mtx_);
    for (auto& it : reliable_out_streams_)
    {
        it.second.reset();
    }
    ro_lock.unlock();
}

/**************************************************************************************************
 * Input Stream Methods.
 **************************************************************************************************/
inline bool Session::next_input_message(InputMessagePtr& message)
{
    bool rv;
    dds::xrce::StreamId stream_id = message->get_header().stream_id();
    SeqNum seq_num = message->get_header().sequence_nr();
    if (0 == stream_id)
    {
        rv = true;
    }
    else if (128 > stream_id)
    {
        std::lock_guard<std::mutex> bi_lock(bi_mtx_);
        rv = besteffort_in_streams_[stream_id].next_message(seq_num);
    }
    else
    {
        std::lock_guard<std::mutex> ri_lock(ri_mtx_);
        rv = reliable_in_streams_[stream_id].next_message(seq_num, message);
    }
    return rv;
}

inline bool Session::pop_input_message(dds::xrce::StreamId stream_id, InputMessagePtr& message)
{
    bool rv = false;
    if (127 < stream_id)
    {
        std::lock_guard<std::mutex> ri_lock(ri_mtx_);
        rv = reliable_in_streams_[stream_id].pop_message(message);
    }
    return rv;
}

inline void Session::update_from_heartbeat(dds::xrce::StreamId stream_id, SeqNum first_unacked, SeqNum last_unacked)
{
    if (127 < stream_id)
    {
        std::lock_guard<std::mutex> ri_lock(ri_mtx_);
        reliable_in_streams_[stream_id].update_from_heartbeat(first_unacked, last_unacked);
    }
}

inline SeqNum Session::get_first_unacked_seq_num(dds::xrce::StreamId stream_id)
{
    std::lock_guard<std::mutex> ri_lock(ri_mtx_);
    return reliable_in_streams_[stream_id].get_first_unacked();
}

inline std::array<uint8_t, 2> Session::get_nack_bitmap(const dds::xrce::StreamId stream_id)
{
    std::array<uint8_t, 2> bitmap = {0, 0};
    if (127 < stream_id)
    {
        std::lock_guard<std::mutex> ri_lock(ri_mtx_);
        bitmap = reliable_in_streams_[stream_id].get_nack_bitmap();
    }
    return bitmap;
}

inline void Session::push_input_fragment(dds::xrce::StreamId stream_id, InputMessagePtr& message)
{
    if (127 < stream_id)
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
inline bool Session::push_output_message(dds::xrce::StreamId stream_id, OutputMessagePtr& output_message)
{
    bool rv = false;
    if (128 > stream_id)
    {
        std::lock_guard<std::mutex> bo_lock(bo_mtx_);
        besteffort_out_streams_.at(stream_id).promote_stream();
        rv = true;
    }
    else
    {
        std::lock_guard<std::mutex> ro_lock(ro_mtx_);
        rv = reliable_out_streams_.at(stream_id).push_message(output_message);
    }
    return rv;
}

inline bool Session::get_output_message(dds::xrce::StreamId stream_id, SeqNum seq_num, OutputMessagePtr& output_message)
{
    bool rv = false;
    if (127 < stream_id)
    {
        std::lock_guard<std::mutex> ro_lock(ro_mtx_);
        rv = reliable_out_streams_.at(stream_id).get_message(seq_num, output_message);
    }
    return rv;
}

inline SeqNum Session::get_first_unacked_seq_nr(const dds::xrce::StreamId stream_id)
{
    std::lock_guard<std::mutex> ro_lock(ro_mtx_);
    return (127 < stream_id) ? reliable_out_streams_.at(stream_id).get_first_available() : SeqNum(0);
}

inline SeqNum Session::get_last_unacked_seq_nr(const dds::xrce::StreamId stream_id)
{
    std::lock_guard<std::mutex> ro_lock(ro_mtx_);
    return (127 < stream_id) ? reliable_out_streams_.at(stream_id).get_last_available() : SeqNum(0);
}

inline void Session::update_from_acknack(const dds::xrce::StreamId stream_id, const SeqNum first_unacked)
{
    if (127 < stream_id)
    {
        std::lock_guard<std::mutex> ro_lock(ro_mtx_);
        reliable_out_streams_.at(stream_id).update_from_acknack(first_unacked);
    }
}

//inline SeqNum Session::next_output_message(const dds::xrce::StreamId stream_id)
//{
//    SeqNum rv;
//    if (128 > stream_id)
//    {
//        std::lock_guard<std::mutex> bo_lock(bo_mtx_);
//        rv = besteffort_out_streams_.at(stream_id).get_last_handled() + 1;
//    }
//    else
//    {
//        std::lock_guard<std::mutex> ro_lock(ro_mtx_);
//        rv = reliable_out_streams_.at(stream_id).next_message();
//    }
//    return rv;
//}

inline std::vector<uint8_t> Session::get_output_streams()
{
    std::lock_guard<std::mutex> ro_lock(ro_mtx_);
    std::vector<uint8_t> result;
    result.reserve(reliable_out_streams_.size());
    for (auto it = reliable_out_streams_.begin(); it != reliable_out_streams_.end(); ++it)
    {
        result.push_back(it->first);
    }
    return result;
}

inline bool Session::message_pending(const dds::xrce::StreamId stream_id)
{
    std::lock_guard<std::mutex> ro_lock(ro_mtx_);
    bool result = false;
    if (127 < stream_id)
    {
        result = reliable_out_streams_.at(stream_id).message_pending();
    }
    return result;
}

/* Fragment relate function. */
template<class T>
inline void Session::push_output_submessage(dds::xrce::StreamId stream_id,
                                            dds::xrce::SubmessageId id,
                                            const T& submessage)
{
    if (128 > stream_id)
    {
        std::lock_guard<std::mutex> bo_lock(bo_mtx_);
        besteffort_out_streams_.at(stream_id).push_submessage(id, submessage);
    }
    else
    {
        std::lock_guard<std::mutex> ro_lock(ro_mtx_);
        reliable_out_streams_.at(stream_id).push_submessage(id, submessage);
    }
}

inline bool Session::get_next_output_message(dds::xrce::StreamId stream_id, OutputMessagePtr& output_message)
{
    bool rv = false;
    if (128 > stream_id)
    {
        std::lock_guard<std::mutex> bo_lock(bo_mtx_);
        rv = besteffort_out_streams_.at(stream_id).get_next_message(output_message);
    }
    else
    {
        std::lock_guard<std::mutex> ro_lock(ro_mtx_);
        rv = reliable_out_streams_.at(stream_id).get_next_message(output_message);
    }
    return rv;
}

} // namespace uxr
} // namespace eprosima

#endif //_UXR_AGENT_CLIENT_SESSION_SESSION_HPP_
