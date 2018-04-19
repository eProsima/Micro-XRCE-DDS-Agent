#ifndef XRCESTREAMS_H
#define XRCESTREAMS_H

#include <XRCETypes.h>
#include <mutex>
#include <vector>
#include <map>
#include <limits>

#define STREAM_HISTORY_DEPTH 16
#define SEQ_NUM_LIMITS (1 << 16)

namespace eprosima { namespace micrortps {

typedef struct XrceMessage
{
    char* buf;
    size_t len;

} XrceMessage;

inline uint16_t add_seq_num(uint16_t s, uint16_t n)
{
    return (s + n) % SEQ_NUM_LIMITS;
}

inline bool seq_num_is_less(uint16_t a, uint16_t b)
{
    bool result = false;
    if (a != b)
    {
        if (((a < b) && ((b - a) < (SEQ_NUM_LIMITS >> 1))) || ((a > b) && ((a - b) > (SEQ_NUM_LIMITS >> 1))))
        {
            result = true;
        }
    }
    return result;
}

inline bool seq_num_is_greater(uint16_t a, uint16_t b)
{
    bool result = false;
    if (a != b)
    {
        if (((a < b) && ((b - a) > (SEQ_NUM_LIMITS >> 1))) || ((a > b) && ((a - b) < (SEQ_NUM_LIMITS >> 1))))
        {
            result = true;
        }
    }
    return result;
}

class BestEffortStream
{
public:
    BestEffortStream() : last_handled_(0xFFFF) {}

    void update(uint16_t seq_num);
    uint16_t get_last_handled() const { return last_handled_; }
    bool is_valid(uint16_t seq_num);

private:
    uint16_t last_handled_;
};

class ReliableInputStream
{
public:
    ReliableInputStream() : last_handled_(0xFFFF), last_announced_(0xFFFF) {}

    ReliableInputStream(const ReliableInputStream&) = delete;
    ReliableInputStream& operator=(const ReliableInputStream) = delete;
    ReliableInputStream(ReliableInputStream&&);
    ReliableInputStream& operator=(ReliableInputStream&&);

    void insert_message(uint16_t index, const char* buf, size_t len);
    bool message_available();
    XrceMessage get_next_message();
    void update_from_heartbeat(uint16_t first_available, uint16_t last_available);
    void update_from_message(uint16_t seq_num);
    uint16_t get_first_unacked() const;
    std::array<uint8_t, 2> get_nack_bitmap();
    bool is_valid(uint16_t seq_num);

private:
    uint16_t last_handled_;
    uint16_t last_announced_;
    std::map<uint16_t, std::vector<uint8_t>> messages_;
    std::mutex mtx_;
};

class ReliableOutputStream
{
public:
    ReliableOutputStream() : last_sent_(0xFFFF), last_acknown_(0xFFFF) {}

    ReliableOutputStream(const ReliableOutputStream&) = delete;
    ReliableOutputStream& operator=(const ReliableOutputStream) = delete;
    ReliableOutputStream(ReliableOutputStream&&);
    ReliableOutputStream& operator=(ReliableOutputStream&&);


    void push_message(const char* buf, size_t len);
    XrceMessage get_message(uint16_t index);
    void update_from_acknack(uint16_t first_unacked);
    uint16_t get_first_available() { return add_seq_num(last_acknown_, 1); }
    uint16_t get_last_available() { return last_sent_; }
    uint16_t next_message() { return add_seq_num(last_sent_, 1); }
    bool message_pending() { return messages_.size() != 0; }

private:
    uint16_t last_sent_;
    uint16_t last_acknown_;
    std::map<uint16_t, std::vector<uint8_t>> messages_;
    std::mutex mtx_;
};

} } // namespace eprosima::micrortps

#endif // XRCESTREAMS_H
