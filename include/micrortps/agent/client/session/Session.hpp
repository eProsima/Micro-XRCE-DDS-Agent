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

#ifndef _MICRORTPS_AGENT_CLIENT_SESSION_SESSION_HPP_
#define _MICRORTPS_AGENT_CLIENT_SESSION_SESSION_HPP_

#include <micrortps/agent/client/session/stream/InputStream.hpp>
#include <micrortps/agent/client/session/stream/OutputStream.hpp>

#include <memory>
#include <map>

namespace eprosima {
namespace micrortps {

class Session
{
public:
    Session() = default;
    ~Session() = default;

    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;
    Session(Session&&) = default;
    Session& operator=(Session&&) = default;

    /* Input streams functions. */
    bool next_input_message(dds::xrce::StreamId stream_id, SeqNum seq_num, const XrceMessage& message);
    bool pop_input_message(dds::xrce::StreamId stream_id, XrceMessage& message);
    void update_from_heartbeat(dds::xrce::StreamId stream_id, SeqNum first_unacked, SeqNum last_unacked);
    SeqNum get_first_unacked_seq_num(dds::xrce::StreamId stream_id);
    std::array<uint8_t, 2> get_nack_bitmap(dds::xrce::StreamId stream_id);
    void remove_last_message(dds::xrce::StreamId stream_id);

    /* Output streams functions. */
    void push_output_message(dds::xrce::StreamId stream_id, const XrceMessage& message);
    XrceMessage get_output_message(dds::xrce::StreamId stream_id, SeqNum index);
    SeqNum get_first_unacked_seq_nr(dds::xrce::StreamId stream_id);
    SeqNum get_last_unacked_seq_nr(dds::xrce::StreamId stream_id);
    void update_from_acknack(dds::xrce::StreamId stream_id, SeqNum first_unacked);
    SeqNum next_ouput_message(dds::xrce::StreamId stream_id);
    std::vector<uint8_t> get_output_streams();
    bool message_pending(dds::xrce::StreamId stream_id);

private:
    std::map<dds::xrce::StreamId, BestEffortInputStream> input_best_effort_streams_;
    std::map<dds::xrce::StreamId, ReliableInputStream> input_relible_streams_;
    std::map<dds::xrce::StreamId, BestEffortOutputStream> output_best_effort_streams_;
    std::map<dds::xrce::StreamId, ReliableOutputStream> output_relible_streams_;
};

/**************************************************************************************************
 * Input Stream Methods.
 **************************************************************************************************/
inline bool Session::next_input_message(dds::xrce::StreamId stream_id, SeqNum seq_num, const XrceMessage& message)
{
    bool rv;
    if (0 == stream_id)
    {
        rv = true;
    }
    else if (128 > stream_id)
    {
        rv = input_best_effort_streams_[stream_id].next_message(seq_num);
    }
    else
    {
        rv = input_relible_streams_[stream_id].next_message(seq_num, message);
    }
    return rv;
}

inline bool Session::pop_input_message(dds::xrce::StreamId stream_id, XrceMessage& message)
{
    bool rv = false;
    if (127 < stream_id)
    {
        rv = input_relible_streams_[stream_id].pop_message(message);
    }
    return rv;
}

inline void Session::update_from_heartbeat(dds::xrce::StreamId stream_id, SeqNum first_unacked, SeqNum last_unacked)
{
    if (127 < stream_id)
    {
        input_relible_streams_[stream_id].update_from_heartbeat(first_unacked, last_unacked);
    }
}

inline SeqNum Session::get_first_unacked_seq_num(dds::xrce::StreamId stream_id)
{
    return input_relible_streams_[stream_id].get_first_unacked();
}

inline void Session::remove_last_message(dds::xrce::StreamId stream_id)
{
    if (127 < stream_id)
    {
        input_relible_streams_[stream_id].remove_last_message();
    }
}

/**************************************************************************************************
 * Output Stream Methods.
 **************************************************************************************************/
inline void Session::push_output_message(dds::xrce::StreamId stream_id, const XrceMessage& message)
{
    if (128 > stream_id)
    {
        output_best_effort_streams_[stream_id].promote_stream();
    }
    else
    {
        output_relible_streams_[stream_id].push_message(message);
    }
}

inline XrceMessage Session::get_output_message(dds::xrce::StreamId stream_id, SeqNum index)
{
    XrceMessage message = {0x00, 0x00};
    if (127 < stream_id)
    {
        message = output_relible_streams_[stream_id].get_message(index);
    }
    return message;
}

inline std::array<uint8_t, 2> Session::get_nack_bitmap(const dds::xrce::StreamId stream_id)
{
    std::array<uint8_t, 2> bitmap = {0, 0};
    if (127 < stream_id)
    {
        bitmap = input_relible_streams_[stream_id].get_nack_bitmap();
    }
    return bitmap;
}

inline SeqNum Session::get_first_unacked_seq_nr(const dds::xrce::StreamId stream_id)
{
    return (127 < stream_id) ? output_relible_streams_[stream_id].get_first_available() : SeqNum(0);
}

inline SeqNum Session::get_last_unacked_seq_nr(const dds::xrce::StreamId stream_id)
{
    return (127 < stream_id) ? output_relible_streams_[stream_id].get_last_available() : SeqNum(0);
}

inline void Session::update_from_acknack(const dds::xrce::StreamId stream_id, const SeqNum first_unacked)
{
    if (127 < stream_id)
    {
        output_relible_streams_[stream_id].update_from_acknack(first_unacked);
    }
}

inline SeqNum Session::next_ouput_message(const dds::xrce::StreamId stream_id)
{
    SeqNum rv;
    if (128 > stream_id)
    {
        rv = output_best_effort_streams_[stream_id].get_last_handled() + 1;
    }
    else
    {
        rv = output_relible_streams_[stream_id].next_message();
    }
    return rv;
}

inline std::vector<uint8_t> Session::get_output_streams()
{
    std::vector<uint8_t> result;
    result.reserve(output_relible_streams_.size());
    for (auto it = output_relible_streams_.begin(); it != output_relible_streams_.end(); ++it)
    {
        result.push_back(it->first);
    }
    return result;
}

inline bool Session::message_pending(const dds::xrce::StreamId stream_id)
{
    bool result = false;
    if (127 < stream_id)
    {
        result = output_relible_streams_[stream_id].message_pending();
    }
    return result;
}

} // namespace micrortps
} // namespace eprosima

#endif //_MICRORTPS_AGENT_CLIENT_SESSION_SESSION_HPP_
