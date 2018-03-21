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
    while (seq_num_ < first_unacked)
    {
        messages_.erase(seq_num_);
        seq_num_++;
    }

    /* Update acknack number. */
    if (ack_num_ <= last_unacked)
    {
        ack_num_ = last_unacked;
    }
}

void ReliableStream::insert_input_message(uint16_t seq_num, const char* buf, size_t len)
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

void ReliableStream::append_output_message(const char* buf, size_t len)
{
    /* Insert message. */
    messages_.emplace(std::piecewise_construct,
                      std::forward_as_tuple(ack_num_),
                      std::forward_as_tuple(buf, buf + len));

    /* Update acknack and sequence number. */
    ack_num_++;
    if (seq_num_ < (ack_num_ - 16))
    {
        seq_num_ = ack_num_ - 16;
    }
}

} } // namespace eprosima::micrortps
