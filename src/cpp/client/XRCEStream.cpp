#include <agent/client/XRCEStreams.h>

namespace eprosima { namespace micrortps {

void BestEffortStream::update_seq_num(uint16_t seq_num)
{
    if (seq_num >= next_seq_num_)
    {
        next_seq_num_ = (seq_num + 1);
    }
}

void ReliableStream::update_seq_num(uint16_t seq_num)
{
    if (seq_num == next_seq_num_)
    {
        next_seq_num_++;
        nack_bitmap_ = nack_bitmap_ >> 1;
    }
}

void ReliableStream::insert_message(uint16_t seq_num, const char* buf, size_t len)
{
    if ((seq_num >= next_seq_num_) && (seq_num < next_seq_num_ + 16))
    {
        messages_.emplace(std::piecewise_construct,
                          std::forward_as_tuple(seq_num),
                          std::forward_as_tuple(buf, buf + len));
    }
}

} }
