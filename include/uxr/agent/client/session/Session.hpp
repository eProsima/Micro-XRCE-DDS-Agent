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

class Session
{
public:
    Session() = default;
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

    /* Output streams functions. */
    void push_output_message(dds::xrce::StreamId stream_id, OutputMessagePtr& output_message);
    bool get_output_message(dds::xrce::StreamId stream_id, SeqNum seq_num, OutputMessagePtr& output_submessage);
    SeqNum get_first_unacked_seq_nr(dds::xrce::StreamId stream_id);
    SeqNum get_last_unacked_seq_nr(dds::xrce::StreamId stream_id);
    void update_from_acknack(dds::xrce::StreamId stream_id, SeqNum first_unacked);
    SeqNum next_output_message(dds::xrce::StreamId stream_id);
    std::vector<uint8_t> get_output_streams();
    bool message_pending(dds::xrce::StreamId stream_id);

private:
    std::unordered_map<dds::xrce::StreamId, BestEffortInputStream> besteffort_istreams_;
    std::unordered_map<dds::xrce::StreamId, ReliableInputStream> relible_istreams_;
    std::unordered_map<dds::xrce::StreamId, BestEffortOutputStream> besteffort_ostreams_;
    std::unordered_map<dds::xrce::StreamId, ReliableOutputStream> relible_ostreams_;
    std::mutex bi_mtx_;
    std::mutex ri_mtx_;
    std::mutex bo_mtx_;
    std::mutex ro_mtx_;
};

inline void Session::reset()
{

    /* Reset Best-Effor Input streams. */
    std::unique_lock<std::mutex> bi_lock(bi_mtx_);
    for (auto& it : besteffort_istreams_)
    {
        it.second.reset();
    }
    bi_lock.unlock();

    /* Reset Reliable Input streams. */
    std::unique_lock<std::mutex> ri_lock(ri_mtx_);
    for (auto& it : relible_istreams_)
    {
        it.second.reset();
    }
    ri_lock.unlock();

    /* Reset Best-Effor Output streams. */
    std::unique_lock<std::mutex> bo_lock(bo_mtx_);
    for (auto& it : besteffort_ostreams_)
    {
        it.second.reset();
    }
    bo_lock.unlock();

    /* Reset Reliable Output streams. */
    std::unique_lock<std::mutex> ro_lock(ro_mtx_);
    for (auto& it : relible_ostreams_)
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
        rv = besteffort_istreams_[stream_id].next_message(seq_num);
    }
    else
    {
        std::lock_guard<std::mutex> ri_lock(ri_mtx_);
        rv = relible_istreams_[stream_id].next_message(seq_num, message);
    }
    return rv;
}

inline bool Session::pop_input_message(dds::xrce::StreamId stream_id, InputMessagePtr& message)
{
    bool rv = false;
    if (127 < stream_id)
    {
        std::lock_guard<std::mutex> ri_lock(ri_mtx_);
        rv = relible_istreams_[stream_id].pop_message(message);
    }
    return rv;
}

inline void Session::update_from_heartbeat(dds::xrce::StreamId stream_id, SeqNum first_unacked, SeqNum last_unacked)
{
    if (127 < stream_id)
    {
        std::lock_guard<std::mutex> ri_lock(ri_mtx_);
        relible_istreams_[stream_id].update_from_heartbeat(first_unacked, last_unacked);
    }
}

inline SeqNum Session::get_first_unacked_seq_num(dds::xrce::StreamId stream_id)
{
    std::lock_guard<std::mutex> ri_lock(ri_mtx_);
    return relible_istreams_[stream_id].get_first_unacked();
}

inline std::array<uint8_t, 2> Session::get_nack_bitmap(const dds::xrce::StreamId stream_id)
{
    std::array<uint8_t, 2> bitmap = {0, 0};
    if (127 < stream_id)
    {
        std::lock_guard<std::mutex> ri_lock(ri_mtx_);
        bitmap = relible_istreams_[stream_id].get_nack_bitmap();
    }
    return bitmap;
}

/**************************************************************************************************
 * Output Stream Methods.
 **************************************************************************************************/
inline void Session::push_output_message(dds::xrce::StreamId stream_id, OutputMessagePtr& output_message)
{
    if (128 > stream_id)
    {
        std::lock_guard<std::mutex> bo_lock(bo_mtx_);
        besteffort_ostreams_[stream_id].promote_stream();
    }
    else
    {
        std::lock_guard<std::mutex> ro_lock(ro_mtx_);
        relible_ostreams_[stream_id].push_message(output_message);
    }
}

inline bool Session::get_output_message(dds::xrce::StreamId stream_id, SeqNum seq_num, OutputMessagePtr& output_message)
{
    bool rv = false;
    if (127 < stream_id)
    {
        std::lock_guard<std::mutex> ro_lock(ro_mtx_);
        rv = relible_ostreams_[stream_id].get_message(seq_num, output_message);
    }
    return rv;
}

inline SeqNum Session::get_first_unacked_seq_nr(const dds::xrce::StreamId stream_id)
{
    std::lock_guard<std::mutex> ro_lock(ro_mtx_);
    return (127 < stream_id) ? relible_ostreams_[stream_id].get_first_available() : SeqNum(0);
}

inline SeqNum Session::get_last_unacked_seq_nr(const dds::xrce::StreamId stream_id)
{
    std::lock_guard<std::mutex> ro_lock(ro_mtx_);
    return (127 < stream_id) ? relible_ostreams_[stream_id].get_last_available() : SeqNum(0);
}

inline void Session::update_from_acknack(const dds::xrce::StreamId stream_id, const SeqNum first_unacked)
{
    if (127 < stream_id)
    {
        std::lock_guard<std::mutex> ro_lock(ro_mtx_);
        relible_ostreams_[stream_id].update_from_acknack(first_unacked);
    }
}

inline SeqNum Session::next_output_message(const dds::xrce::StreamId stream_id)
{
    SeqNum rv;
    if (128 > stream_id)
    {
        std::lock_guard<std::mutex> bo_lock(bo_mtx_);
        rv = besteffort_ostreams_[stream_id].get_last_handled() + 1;
    }
    else
    {
        std::lock_guard<std::mutex> ro_lock(ro_mtx_);
        rv = relible_ostreams_[stream_id].next_message();
    }
    return rv;
}

inline std::vector<uint8_t> Session::get_output_streams()
{
    std::lock_guard<std::mutex> ro_lock(ro_mtx_);
    std::vector<uint8_t> result;
    result.reserve(relible_ostreams_.size());
    for (auto it = relible_ostreams_.begin(); it != relible_ostreams_.end(); ++it)
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
        result = relible_ostreams_[stream_id].message_pending();
    }
    return result;
}

} // namespace uxr
} // namespace eprosima

#endif //_UXR_AGENT_CLIENT_SESSION_SESSION_HPP_
