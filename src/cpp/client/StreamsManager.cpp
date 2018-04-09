#include <agent/client/StreamsManager.h>

namespace eprosima { namespace micrortps {

StreamsManager::StreamsManager()
    : input_best_effort_streams_(),
      input_relible_streams_(),
      output_best_effort_streams_(),
      output_relible_streams_()
{
}

bool StreamsManager::is_next_message(dds::xrce::StreamId stream_id, uint16_t seq_num)
{
    bool result = false;
    if (0x00 == stream_id)
    {
        result = true;
    }
    else if (128 > stream_id)
    {
        if (seq_num > input_best_effort_streams_[stream_id].get_last_handled())
        {
            result = true;
        }
    }
    else
    {
        if (seq_num == input_relible_streams_[stream_id].get_first_unacked())
        {
            result = true;
        }
    }
    return result;
}

bool StreamsManager::is_valid_message(dds::xrce::StreamId stream_id, uint16_t seq_num)
{
    bool result = false;
    if (0x00 == stream_id)
    {
        result = true;
    }
    else if (128 > stream_id)
    {
        if (seq_num >= input_best_effort_streams_[stream_id].get_last_handled())
        {
            result = true;
        }
    }
    else
    {
        uint16_t first_unacked = input_relible_streams_[stream_id].get_first_unacked();
        if ((seq_num >= first_unacked) && (seq_num < (first_unacked + (uint16_t)16)))
        {
            result = true;
        }
    }
    return result;
}

bool StreamsManager::message_available(dds::xrce::StreamId stream_id)
{
    bool result = false;
    if (127 < stream_id)
    {
        result = input_relible_streams_[stream_id].message_available();
    }
    return result;
}

void StreamsManager::promote_stream(dds::xrce::StreamId stream_id, uint16_t seq_num)
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

void StreamsManager::update_from_heartbeat(dds::xrce::StreamId stream_id, uint16_t first_unacked, uint16_t last_unacked)
{
    if (127 < stream_id)
    {
        input_relible_streams_[stream_id].update_from_heartbeat(first_unacked, last_unacked);
    }
}

dds::xrce::XrceMessage StreamsManager::get_next_message(dds::xrce::StreamId stream_id)
{
    dds::xrce::XrceMessage next_message = {nullptr, 0};
    if (127 < stream_id)
    {
        next_message = input_relible_streams_[stream_id].get_next_message();
    }
    return	next_message;
}

void StreamsManager::store_input_message(dds::xrce::StreamId stream_id, uint16_t seq_num, const char* buf, size_t len)
{
    if (127 < stream_id)
    {
        input_relible_streams_[stream_id].insert_message(seq_num, buf, len);
    }
}

uint16_t StreamsManager::get_first_unacked_seq_num(dds::xrce::StreamId stream_id)
{
    return input_relible_streams_[stream_id].get_first_unacked();
}

void StreamsManager::store_output_message(dds::xrce::StreamId stream_id, const char* buf, size_t len)
{
    if (128 > stream_id)
    {
        uint16_t index = output_best_effort_streams_[stream_id].get_last_handled() + (uint16_t)1;
        output_best_effort_streams_[stream_id].update(index);
    }
    else
    {
        output_relible_streams_[stream_id].push_message(buf, len);
    }
}

dds::xrce::XrceMessage StreamsManager::get_output_message(dds::xrce::StreamId stream_id, uint16_t index)
{
    dds::xrce::XrceMessage message = {0x00, 0x00};
    if (127 < stream_id)
    {
        message = output_relible_streams_[stream_id].get_message(index);
    }
    return message;
}

std::array<uint8_t, 2> StreamsManager::get_nack_bitmap(dds::xrce::StreamId stream_id)
{
    std::array<uint8_t, 2> bitmap = {0, 0};
    if (127 < stream_id)
    {
        bitmap = input_relible_streams_[stream_id].get_nack_bitmap();
    }
    return bitmap;
}

uint16_t StreamsManager::get_first_unacked_seq_nr(dds::xrce::StreamId stream_id)
{
    uint16_t result = (127 < stream_id) ? output_relible_streams_[stream_id].get_first_available() : 0;
    return result;
}

uint16_t StreamsManager::get_last_unacked_seq_nr(dds::xrce::StreamId stream_id)
{
    uint16_t result = (127 < stream_id) ? output_relible_streams_[stream_id].get_last_available() : 0;
    return result;
}

void StreamsManager::update_from_acknack(dds::xrce::StreamId stream_id, uint16_t first_unacked)
{
    if (127 < stream_id)
    {
        output_relible_streams_[stream_id].update_from_acknack(first_unacked);
    }
}

uint16_t StreamsManager::next_ouput_message(dds::xrce::StreamId stream_id)
{
    uint16_t result;
    if (128 > stream_id)
    {
        result = output_best_effort_streams_[stream_id].get_last_handled() + 1;
    }
    if (127 < stream_id)
    {
        result = output_relible_streams_[stream_id].next_message();
    }
    return result;
}

std::vector<uint8_t> StreamsManager::get_output_streams()
{
    std::vector<uint8_t> result;
    for (auto it = output_relible_streams_.begin(); it != output_relible_streams_.end(); ++it)
    {
        result.push_back(it->first);
    }
    return result;
}

bool StreamsManager::message_pending(dds::xrce::StreamId stream_id)
{
    bool result = false;
    if (127 < stream_id)
    {
        result = output_relible_streams_[stream_id].message_pending();
    }
    return result;
}

} } // namespace eprosima::micrortps
