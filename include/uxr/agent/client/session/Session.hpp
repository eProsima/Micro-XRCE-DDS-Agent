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
#include <uxr/agent/utils/SharedMutex.hpp>

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
public:
    Session(const SessionInfo& info)
        : session_info_(info)
        , none_ostream_{}
    {}

    ~Session() = default;

    Session(Session&& x) = delete;
    Session(const Session&) = delete;
    Session& operator=(Session&& x) = delete;
    Session& operator=(const Session&) = delete;

    void reset();

    /* Input streams functions. */
    bool push_input_message(
            InputMessagePtr&& message,
            dds::xrce::StreamId stream_id,
            dds::xrce::SequenceNr sequence_nr);

    bool pop_input_message(
            dds::xrce::StreamId stream_id,
            InputMessagePtr& message);

    void update_from_heartbeat(
            dds::xrce::StreamId stream_id,
            SeqNum first_unacked,
            SeqNum last_unacked);

    void fill_acknack(
            dds::xrce::StreamId stream_id,
            dds::xrce::ACKNACK_Payload& acknack);

    void push_input_fragment(
            dds::xrce::StreamId stream_id,
            InputMessagePtr& message);

    bool pop_input_fragment_message(
            dds::xrce::StreamId stream_id,
            InputMessagePtr& message);

    /* Output streams functions. */
    std::vector<uint8_t> get_output_streams();

    template<class T>
    bool push_output_submessage(
            dds::xrce::StreamId stream_id,
            dds::xrce::SubmessageId submessage_id,
            const T& submessage,
            std::chrono::milliseconds timeout);

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
    ReliableOutputStream& get_reliable_output_stream(
            dds::xrce::StreamId stream_id,
            utils::SharedLock& shared_lock);

private:
    const SessionInfo session_info_;

    NoneInputStream none_istream_;
    std::unordered_map<dds::xrce::StreamId, BestEffortInputStream> best_effort_istreams_;
    std::unordered_map<dds::xrce::StreamId, ReliableInputStream> reliable_istreams_;
    std::mutex best_effort_imtx_;
    std::mutex reliable_imtx_;

    NoneOutputStream none_ostream_;
    std::unordered_map<dds::xrce::StreamId, BestEffortOutputStream> best_effort_ostreams_;
    std::unordered_map<dds::xrce::StreamId, ReliableOutputStream> reliable_ostreams_;
    std::mutex best_effort_omtx_;
    utils::SharedMutex reliable_omtx_;
};

inline void Session::reset()
{
    std::unique_lock<std::mutex> best_effort_ilock(best_effort_imtx_);
    for (auto& it : best_effort_istreams_)
    {
        it.second.reset();
    }
    best_effort_ilock.unlock();

    std::unique_lock<std::mutex> reliable_ilock(reliable_imtx_);
    for (auto& it : reliable_istreams_)
    {
        it.second.reset();
    }
    reliable_ilock.unlock();

    none_ostream_.reset();

    std::unique_lock<std::mutex> best_effort_olock(best_effort_omtx_);
    for (auto& it : best_effort_ostreams_)
    {
        it.second.reset();
    }
    best_effort_olock.unlock();

    utils::SharedLock reliable_olock(reliable_omtx_);
    for (auto& it : reliable_ostreams_)
    {
        it.second.reset();
    }
    reliable_olock.unlock();
}

/**************************************************************************************************
 * Input Stream Methods.
 **************************************************************************************************/
inline bool Session::push_input_message(
        InputMessagePtr&& message,
        dds::xrce::StreamId stream_id,
        dds::xrce::SequenceNr sequence_nr)
{
    bool rv = false;
    SeqNum seq_num{sequence_nr};
    if (is_none_stream(stream_id))
    {
        rv = none_istream_.push_message(std::move(message));
    }
    else if (is_besteffort_stream(stream_id))
    {
        std::lock_guard<std::mutex> lock(best_effort_imtx_);
        rv = best_effort_istreams_[stream_id].push_message(sequence_nr, std::move(message));
    }
    else
    {
        std::lock_guard<std::mutex> lock(reliable_imtx_);
        rv = reliable_istreams_[stream_id].push_message(sequence_nr, std::move(message));
    }
    return rv;
}

inline bool Session::pop_input_message(
        dds::xrce::StreamId stream_id,
        InputMessagePtr& message)
{
    bool rv = false;
    if (is_none_stream(stream_id))
    {
        rv = none_istream_.pop_message(message);
    }
    else if (is_besteffort_stream(stream_id))
    {
        std::lock_guard<std::mutex> lock(best_effort_imtx_);
        rv = best_effort_istreams_[stream_id].pop_message(message);
    }
    else
    {
        std::lock_guard<std::mutex> lock(reliable_imtx_);
        rv = reliable_istreams_[stream_id].pop_message(message);
    }
    return rv;
}

inline void Session::update_from_heartbeat(
        dds::xrce::StreamId stream_id,
        SeqNum first_unacked,
        SeqNum last_unacked)
{
    if (is_reliable_stream(stream_id))
    {
        std::lock_guard<std::mutex> lock(reliable_imtx_);
        reliable_istreams_[stream_id].update_from_heartbeat(first_unacked, last_unacked);
    }
}

inline void Session::fill_acknack(
        dds::xrce::StreamId stream_id,
        dds::xrce::ACKNACK_Payload& acknack)
{
    if (is_reliable_stream(stream_id))
    {
        std::lock_guard<std::mutex> lock(reliable_imtx_);
        reliable_istreams_[stream_id].fill_acknack(acknack);
    }
}

inline void Session::push_input_fragment(dds::xrce::StreamId stream_id, InputMessagePtr& message)
{
    if (is_reliable_stream(stream_id))
    {
        std::lock_guard<std::mutex> lock(reliable_imtx_);
        reliable_istreams_[stream_id].push_fragment(message);
    }
}

inline bool Session::pop_input_fragment_message(dds::xrce::StreamId stream_id, InputMessagePtr& message)
{
    std::lock_guard<std::mutex> lock(reliable_imtx_);
    return reliable_istreams_[stream_id].pop_fragment_message(message);
}

/**************************************************************************************************
 * Output Stream Methods.
 **************************************************************************************************/

inline std::vector<uint8_t> Session::get_output_streams()
{
    utils::SharedLock lock(reliable_omtx_);
    std::vector<uint8_t> result(reliable_ostreams_.size());
    for (auto it = reliable_ostreams_.begin(); it != reliable_ostreams_.end(); ++it)
    {
        result.push_back(it->first);
    }
    return result;
}

template<class T>
inline bool Session::push_output_submessage(
        dds::xrce::StreamId stream_id,
        dds::xrce::SubmessageId submessage_id,
        const T& submessage,
        std::chrono::milliseconds timeout)
{
    bool rv = false;
    if (is_none_stream(stream_id))
    {
        rv = none_ostream_.push_submessage(session_info_, submessage_id, submessage);
    }
    else if (is_besteffort_stream(stream_id))
    {
        std::lock_guard<std::mutex> lock(best_effort_omtx_);
        rv = best_effort_ostreams_[stream_id].push_submessage(session_info_, stream_id, submessage_id, submessage);
    }
    else
    {
        utils::SharedLock shared_lock(reliable_omtx_);
        rv = get_reliable_output_stream(stream_id, shared_lock).push_submessage(
            session_info_, stream_id, submessage_id, submessage, timeout);
    }
    return rv;
}

inline bool Session::get_next_output_message(
        dds::xrce::StreamId stream_id,
        OutputMessagePtr& output_message)
{
    bool rv = false;
    if (is_none_stream(stream_id))
    {
        rv = none_ostream_.pop_message(output_message);
    }
    else if (is_besteffort_stream(stream_id))
    {
        std::lock_guard<std::mutex> lock(best_effort_omtx_);
        rv = best_effort_ostreams_[stream_id].pop_message(output_message);
    }
    else
    {
        utils::SharedLock shared_lock(reliable_omtx_);
        rv = get_reliable_output_stream(stream_id, shared_lock).get_next_message(output_message);
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
        utils::SharedLock shared_lock(reliable_omtx_);
        rv = get_reliable_output_stream(stream_id, shared_lock).get_message(seq_num, output_message);
    }
    return rv;
}

inline void Session::update_from_acknack(
        const dds::xrce::StreamId stream_id,
        const SeqNum first_unacked)
{
    if (is_reliable_stream(stream_id))
    {
        utils::SharedLock shared_lock(reliable_omtx_);
        get_reliable_output_stream(stream_id, shared_lock).update_from_acknack(first_unacked);
    }
}


inline bool Session::fill_heartbeat(
        dds::xrce::StreamId stream_id,
        dds::xrce::HEARTBEAT_Payload& heartbeat)
{
    bool rv = false;
    if (is_reliable_stream(stream_id))
    {
        utils::SharedLock shared_lock(reliable_omtx_);
        rv = get_reliable_output_stream(stream_id, shared_lock).fill_heartbeat(heartbeat);
        heartbeat.stream_id(stream_id);
    }
    return rv;
}

inline ReliableOutputStream& Session::get_reliable_output_stream(
        dds::xrce::StreamId stream_id,
        utils::SharedLock& shared_lock)
{
    shared_lock.lock();
    auto it = reliable_ostreams_.find(stream_id);
    if (it != reliable_ostreams_.end())
    {
        return it->second;
    }
    else
    {
        shared_lock.unlock();
        utils::ExclusiveLock exclusive_lock(reliable_omtx_);
        shared_lock.lock();
        return reliable_ostreams_[stream_id];
    }
}

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_CLIENT_SESSION_SESSION_HPP_
