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
        if (seq_num >= input_best_effort_streams_[stream_id].get_seq_num())
        {
            result = true;
        }
    }
    else
    {
        if (seq_num == input_relible_streams_[stream_id].get_seq_num())
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
        if (seq_num >= input_best_effort_streams_[stream_id].get_seq_num())
        {
            result = true;
        }
    }
    else
    {
        uint16_t next_seq_num = input_relible_streams_[stream_id].get_seq_num();
        if ((seq_num >= next_seq_num) && (seq_num < (next_seq_num + 16)))
        {
            result = true;
        }
    }

    return result;
}

void StreamsManager::promote_seq_num(dds::xrce::StreamId stream_id, uint16_t seq_num)
{
    if (128 > stream_id)
    {
        input_best_effort_streams_[stream_id].promote_seq_num(seq_num);
    }
    else
    {
        input_relible_streams_[stream_id].promote_seq_num(seq_num);
    }
}

void StreamsManager::update_from_heartbeat(dds::xrce::StreamId stream_id, uint16_t first_unacked, uint16_t last_unacked)
{
    if (127 < stream_id)
    {
        input_relible_streams_[stream_id].update_from_heartbeat(first_unacked, last_unacked);
    }
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

dds::xrce::XrceMessage StreamsManager::get_next_message(dds::xrce::StreamId stream_id)
{
    dds::xrce::XrceMessage next_message = {};

    if (127 < stream_id)
    {
        uint16_t seq_num = input_relible_streams_[stream_id].get_seq_num();
        next_message.buf = reinterpret_cast<char*>(input_relible_streams_[stream_id].get_message_data(seq_num));
        next_message.len = input_relible_streams_[stream_id].get_message_size(seq_num);
    }

    return	next_message;
}

void StreamsManager::store_input_message(dds::xrce::StreamId stream_id, uint16_t seq_num, const char* buf, size_t len)
{
    if (127 < stream_id)
    {
        input_relible_streams_[stream_id].insert_input_message(seq_num, buf, len);
    }
}

uint16_t StreamsManager::get_first_unacked_seq_num(dds::xrce::StreamId stream_id)
{
    return input_relible_streams_[stream_id].get_seq_num();
}

uint16_t StreamsManager::get_ack_num(dds::xrce::StreamId stream_id)
{
    uint16_t result;

    if (128 > stream_id)
    {
        result = output_best_effort_streams_[stream_id].get_seq_num();
    }
    else
    {
        result = output_relible_streams_[stream_id].get_ack_num();
    }

    return result;
}

void StreamsManager::store_output_message(dds::xrce::StreamId stream_id, const char* buf, size_t len)
{
    if (127 < stream_id)
    {
        output_relible_streams_[stream_id].append_output_message(buf, len);
    }
}

dds::xrce::XrceMessage StreamsManager::get_output_message(dds::xrce::StreamId stream_id, uint16_t index)
{
    dds::xrce::XrceMessage message;

    if (127 < stream_id)
    {
        message.buf = reinterpret_cast<char*>(output_relible_streams_[stream_id].get_message_data(index));
        message.len = output_relible_streams_[stream_id].get_message_size(index);
    }

    return message;
}

std::array<uint8_t, 2> StreamsManager::get_nack_bitmap(dds::xrce::StreamId stream_id)
{
    std::array<uint8_t, 2> bitmap = {0, 0};

    if (127 < stream_id)
    {
        ReliableStream stream = input_relible_streams_[stream_id];
        uint16_t seq_num = stream.get_seq_num();
        uint16_t ack_num = stream.get_ack_num();
        for (int i = 0; i < 8; ++i)
        {
            if (((seq_num + i) <= ack_num) && (stream.get_message_size(seq_num + i) > 0))
            {
                bitmap.at(1) = bitmap.at(1) | (0x01 << i);
            }
            if ((((seq_num + i + 8) <= ack_num) && stream.get_message_size(seq_num + i + 8) > 0))
            {
                bitmap.at(0) = bitmap.at(0) | (0x01 << i);
            }
        }
    }

    return bitmap;
}

uint16_t StreamsManager::get_first_unacked(dds::xrce::StreamId stream_id)
{
    uint16_t result = (127 < stream_id) ? output_relible_streams_[stream_id].get_seq_num() : 0;
    return result;
}

uint16_t StreamsManager::get_last_unacked(dds::xrce::StreamId stream_id)
{
    uint16_t result = (127 < stream_id) ? output_relible_streams_[stream_id].get_ack_num() - 1 : 0;
    return result;
}

} } // namespace eprosima::micrortps
