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

#include <micrortps/agent/client/StreamsManager.hpp>

namespace eprosima { namespace micrortps {

StreamsManager::StreamsManager()
    : input_best_effort_streams_(),
      input_relible_streams_(),
      output_best_effort_streams_(),
      output_relible_streams_()
{
}

bool StreamsManager::is_next_message(const dds::xrce::StreamId stream_id, const SeqNum seq_num)
{
    bool rv = false;
    if (0x00 == stream_id)
    {
        rv = true;
    }
    else if (128 > stream_id)
    {
        rv = (seq_num > input_best_effort_streams_[stream_id].get_last_handled());
    }
    else
    {
        rv = (seq_num == input_relible_streams_[stream_id].get_first_unacked());
    }
    return rv;
}

bool StreamsManager::is_valid_message(const dds::xrce::StreamId stream_id, const SeqNum seq_num)
{
    bool rv = false;
    if (0x00 == stream_id)
    {
        rv = true;
    }
    else if (128 > stream_id)
    {
        rv = (seq_num > input_best_effort_streams_[stream_id].get_last_handled());
    }
    else
    {
        SeqNum first_unacked = input_relible_streams_[stream_id].get_first_unacked();
        rv = ((seq_num >= first_unacked) && (seq_num < 16 + first_unacked));
    }
    return rv;
}

bool StreamsManager::message_available(const dds::xrce::StreamId stream_id)
{
    bool rv = false;
    if (127 < stream_id)
    {
        rv = input_relible_streams_[stream_id].message_available();
    }
    return rv;
}

void StreamsManager::promote_stream(const dds::xrce::StreamId stream_id, const SeqNum seq_num)
{
    if (128 > stream_id)
    {
        input_best_effort_streams_[stream_id].update(seq_num);
    }
    else
    {
        input_relible_streams_[stream_id].update_from_message(seq_num);
    }
}

void StreamsManager::update_from_heartbeat(const dds::xrce::StreamId stream_id,
                                           const SeqNum first_unacked,
                                           const SeqNum last_unacked)
{
    if (127 < stream_id)
    {
        input_relible_streams_[stream_id].update_from_heartbeat(first_unacked, last_unacked);
    }
}

XrceMessage StreamsManager::get_next_message(const dds::xrce::StreamId stream_id)
{
    XrceMessage next_message = {nullptr, 0};
    if (127 < stream_id)
    {
        next_message = input_relible_streams_[stream_id].get_next_message();
    }
    return	next_message;
}

void StreamsManager::store_input_message(const dds::xrce::StreamId stream_id,
                                         const SeqNum seq_num,
                                         const char* buf, size_t len)
{
    if (127 < stream_id)
    {
        input_relible_streams_[stream_id].insert_message(seq_num, buf, len);
    }
}

SeqNum StreamsManager::get_first_unacked_seq_num(const dds::xrce::StreamId stream_id)
{
    return input_relible_streams_[stream_id].get_first_unacked();
}

void StreamsManager::store_output_message(const dds::xrce::StreamId stream_id, const char* buf, size_t len)
{
    if (128 > stream_id)
    {
        SeqNum index = output_best_effort_streams_[stream_id].get_last_handled() + 1;
        output_best_effort_streams_[stream_id].update(index);
    }
    else
    {
        output_relible_streams_[stream_id].push_message(buf, len);
    }
}

XrceMessage StreamsManager::get_output_message(const dds::xrce::StreamId stream_id, const SeqNum index)
{
    XrceMessage message = {0x00, 0x00};
    if (127 < stream_id)
    {
        message = output_relible_streams_[stream_id].get_message(index);
    }
    return message;
}

std::array<uint8_t, 2> StreamsManager::get_nack_bitmap(const dds::xrce::StreamId stream_id)
{
    std::array<uint8_t, 2> bitmap = {0, 0};
    if (127 < stream_id)
    {
        bitmap = input_relible_streams_[stream_id].get_nack_bitmap();
    }
    return bitmap;
}

SeqNum StreamsManager::get_first_unacked_seq_nr(const dds::xrce::StreamId stream_id)
{
    return (127 < stream_id) ? output_relible_streams_[stream_id].get_first_available() : SeqNum(0);
}

SeqNum StreamsManager::get_last_unacked_seq_nr(const dds::xrce::StreamId stream_id)
{
    return (127 < stream_id) ? output_relible_streams_[stream_id].get_last_available() : SeqNum(0);
}

void StreamsManager::update_from_acknack(const dds::xrce::StreamId stream_id, const SeqNum first_unacked)
{
    if (127 < stream_id)
    {
        output_relible_streams_[stream_id].update_from_acknack(first_unacked);
    }
}

SeqNum StreamsManager::next_ouput_message(const dds::xrce::StreamId stream_id)
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

std::vector<uint8_t> StreamsManager::get_output_streams()
{
    std::vector<uint8_t> result;
    result.reserve(output_relible_streams_.size());
    for (auto it = output_relible_streams_.begin(); it != output_relible_streams_.end(); ++it)
    {
        result.push_back(it->first);
    }
    return result;
}

bool StreamsManager::message_pending(const dds::xrce::StreamId stream_id)
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
