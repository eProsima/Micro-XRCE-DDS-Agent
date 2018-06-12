#include <agent/client/XRCEStreams.h>

namespace eprosima { namespace micrortps {

void BestEffortStream::update(uint16_t seq_num)
{
    if (seq_num_is_greater(seq_num, last_handled_))
    {
        last_handled_ = seq_num;
    }
}

bool BestEffortStream::is_valid(uint16_t seq_num)
{
    return seq_num_is_greater(add_seq_num(seq_num, 1), last_handled_);
}

ReliableInputStream::ReliableInputStream(ReliableInputStream&& x)
    : last_handled_(x.last_handled_),
      last_announced_(x.last_announced_),
      messages_(std::move(x.messages_))
{
}

ReliableInputStream& ReliableInputStream::operator=(ReliableInputStream&& x)
{
    last_handled_ = x.last_handled_;
    last_announced_ = x.last_announced_;
    messages_ = std::move(x.messages_);
    return *this;
}

void ReliableInputStream::insert_message(uint16_t index, const char* buf, size_t len)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if ((seq_num_is_greater(index, last_handled_) && seq_num_is_less(index, last_handled_ + STREAM_HISTORY_DEPTH))
            || index == last_handled_ + STREAM_HISTORY_DEPTH)
    {
        /* Update last_announced. */
        if (seq_num_is_greater(index, last_announced_))
        {
            last_announced_ = index;
        }

        /* Insert element. */
        auto it = messages_.find(index);
        if (it == messages_.end())
        {
            messages_.emplace(std::piecewise_construct,
                              std::forward_as_tuple(index),
                              std::forward_as_tuple(buf, buf + len));
        }
    }
}

bool ReliableInputStream::message_available()
{
    auto it = messages_.find(add_seq_num(last_handled_, 1));
    return (it != messages_.end());
}

XrceMessage ReliableInputStream::get_next_message()
{
    std::lock_guard<std::mutex> lock(mtx_);
    XrceMessage message = {nullptr, 0};
    uint16_t index = add_seq_num(last_handled_, 1);
    auto it = messages_.find(index);
    if (it != messages_.end())
    {
        message.buf = reinterpret_cast<char*>(messages_.at(index).data());
        message.len = messages_.at(index).size();
    }
    return message;
}

void ReliableInputStream::update_from_heartbeat(uint16_t first_available, uint16_t last_available)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (seq_num_is_less(add_seq_num(last_handled_, 1), first_available))
    {
        last_handled_ = first_available;
    }
    if (seq_num_is_less(last_announced_, last_available))
    {
        last_announced_ = last_available;
    }
}

void ReliableInputStream::update_from_message(uint16_t seq_num)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (seq_num == add_seq_num(last_handled_, 1))
    {
        last_handled_ = add_seq_num(last_handled_, 1);
        messages_.erase(last_handled_);
    }
    if (seq_num_is_less(last_announced_, seq_num))
    {
        last_announced_ = seq_num;
    }
}

uint16_t ReliableInputStream::get_first_unacked() const
{
    return add_seq_num(last_handled_, 1);
}

std::array<uint8_t, 2> ReliableInputStream::get_nack_bitmap()
{
    std::lock_guard<std::mutex> lock(mtx_);
    std::array<uint8_t, 2> bitmap = {0, 0};
    for (uint16_t i = 0; i < 8; i++)
    {
        if (seq_num_is_less(add_seq_num(last_handled_, i), last_announced_))
        {
            auto it = messages_.find(add_seq_num(last_handled_, i + 1));
            if (it == messages_.end())
            {
                bitmap.at(1) = bitmap.at(1) | (0x01 << i);
            }
        }
        if (seq_num_is_less(add_seq_num(last_handled_, i + 8), last_announced_))
        {
            auto it = messages_.find(add_seq_num(last_handled_, i + 9));
            if (it == messages_.end())
            {
                bitmap.at(0) = bitmap.at(0) | (0x01 << i);
            }
        }
    }
    return bitmap;
}

ReliableOutputStream::ReliableOutputStream(ReliableOutputStream&& x)
    : last_sent_(x.last_sent_),
      last_acknown_(x.last_acknown_),
      messages_(std::move(messages_))
{
}

ReliableOutputStream& ReliableOutputStream::operator=(ReliableOutputStream&& x)
{
    last_sent_ = x.last_sent_;
    last_acknown_ = x.last_acknown_;
    messages_ = std::move(x.messages_);
    return *this;
}

void ReliableOutputStream::push_message(const char* buf, size_t len)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (seq_num_is_less(last_sent_, add_seq_num(last_acknown_, 16)))
    {
        last_sent_ = add_seq_num(last_sent_, 1);
        messages_.emplace(std::piecewise_construct,
                          std::forward_as_tuple(last_sent_),
                          std::forward_as_tuple(buf, buf + len));
    }
}

XrceMessage ReliableOutputStream::get_message(uint16_t index)
{
    std::lock_guard<std::mutex> lock(mtx_);
    XrceMessage message = {nullptr, 0};
    auto it = messages_.find(index);
    if (it != messages_.end())
    {
        message.buf = reinterpret_cast<char*>(messages_.at(index).data());
        message.len = messages_.at(index).size();
    }
    return message;
}

void ReliableOutputStream::update_from_acknack(uint16_t first_unacked)
{
    std::lock_guard<std::mutex> lock(mtx_);
    while (seq_num_is_less(add_seq_num(last_acknown_, 1), first_unacked) && seq_num_is_less(last_acknown_, last_sent_))
    {
        last_acknown_ = add_seq_num(last_acknown_, 1);
        messages_.erase(last_acknown_);
    }
}

} } // namespace eprosima::micrortps
