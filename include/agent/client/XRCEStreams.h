#ifndef XRCESTREAMS_H
#define XRCESTREAMS_H

#include <vector>
#include <map>

namespace eprosima { namespace micrortps {

class XRCEStream
{
public:
    XRCEStream() : seq_num_(0) {}

    uint16_t get_next_seq_num() { return seq_num_; }
    virtual void promote_seq_num(uint16_t seq_num) = 0;

protected:
    uint16_t seq_num_;
};

class BestEffortStream : public XRCEStream
{
public:
    BestEffortStream() : XRCEStream() {}

    void promote_seq_num(uint16_t seq_num) override;
};

class ReliableStream : public XRCEStream
{
public:
    ReliableStream()
        : XRCEStream(),
          ack_num_(0),
          messages_() {}

    void promote_seq_num(uint16_t seq_num) override;
    void update_from_heartbeat(uint16_t first_unacked, uint16_t last_unacked);
    void insert_message(uint16_t seq_num, const char* buf, size_t len);
    uint8_t* get_next_message_data() { return messages_[seq_num_].data(); }
    size_t get_next_message_length() { return messages_[seq_num_].size(); }
    bool message_available();

private:
    uint16_t ack_num_;
    std::map<uint16_t, std::vector<uint8_t>> messages_;
};

} } // namespace eprosima::micrortps

#endif // XRCESTREAMS_H
