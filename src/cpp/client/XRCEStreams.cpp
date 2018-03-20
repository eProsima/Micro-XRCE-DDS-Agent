#include <agent/client/XRCEStreams.h>

namespace eprosima { namespace micrortps {

void BestEffortStream::promote_seq_num(uint16_t seq_num)
{
    if (seq_num >= seq_num_)
    {
        seq_num_ = (seq_num + 1);
    }
}

void ReliableStream::promote_seq_num(uint16_t seq_num)
{
    if (seq_num == seq_num_)
    {
        messages_.erase(seq_num);
        seq_num_++;
    }
}

void ReliableStream::update_from_heartbeat(uint16_t first_unacked, uint16_t last_unacked)
{
    /* Update sequence number. */
    if (first_unacked > seq_num_)
    {
        seq_num_ = first_unacked;
    }

    /* Update acknack number. */
    if (ack_num_ <= last_unacked)
    {
        ack_num_ = last_unacked;
    }
}

void ReliableStream::insert_message(uint16_t seq_num, const char* buf, size_t len)
{
    if ((seq_num >= seq_num_) && (seq_num < seq_num_ + 16))
    {
        /* Update acknack number. */
        if (ack_num_ <= seq_num)
        {
            ack_num_ = seq_num;
        }

        /* Insert element. */
        auto it = messages_.find(seq_num);
        if (it == messages_.end())
        {
            messages_.emplace(std::piecewise_construct,
                              std::forward_as_tuple(seq_num),
                              std::forward_as_tuple(buf, buf + len));
        }
    }
}

bool ReliableStream::message_available()
{
    return (messages_.begin()->first == seq_num_);
}

} } // namespace eprosima::micrortps
