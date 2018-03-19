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

    if (128 > stream_id)
    {
        if (seq_num >= input_best_effort_streams_[stream_id].get_next_seq_num())
        {
            result = true;
        }
    }
    else
    {
        if (seq_num == input_relible_streams_[stream_id].get_next_seq_num())
        {
            result = true;
        }
    }

    return result;
}

bool StreamsManager::is_valid_message(dds::xrce::StreamId stream_id, uint16_t seq_num)
{
    bool result = false;

    if (128 > stream_id)
    {
        if (seq_num >= input_best_effort_streams_[stream_id].get_next_seq_num())
        {
            result = true;
        }
    }
    else
    {
        uint16_t next_seq_num = input_relible_streams_[stream_id].get_next_seq_num();
        if ((seq_num >= next_seq_num) && (seq_num < (next_seq_num + 16)))
        {
            result = true;
        }
    }

    return result;
}

void StreamsManager::store_message(dds::xrce::StreamId stream_id,
                                   uint16_t seq_num,
                                   const char *buf,
                                   size_t len)
{
    if (127 < stream_id)
    {
        input_relible_streams_[stream_id].insert_message(seq_num, buf, len);
    }
}

} } // namespace eprosima::micrortps
