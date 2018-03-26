#ifndef XRCESTREAMS_H
#define XRCESTREAMS_H

#include <vector>
#include <map>

namespace eprosima { namespace micrortps {

class XRCEStream
{
public:
    XRCEStream() : seq_num_(0) {}

    uint16_t get_seq_num() { return seq_num_; }
    virtual void update(uint16_t seq_num) = 0;

protected:
    uint16_t seq_num_;
};

class BestEffortStream : public XRCEStream
{
public:
    BestEffortStream() : XRCEStream() {}

    void update(uint16_t seq_num) override;
};

class ReliableStream : public XRCEStream
{
public:
    ReliableStream()
        : XRCEStream(),
          ack_num_(0),
          messages_() {}

    void update(uint16_t seq_num) override;
    void update_from_heartbeat(uint16_t first_unacked, uint16_t last_unacked);
    void insert_input_message(uint16_t seq_num, const char* buf, size_t len);
    void append_output_message(const char* buf, size_t len);
    uint8_t* get_message_data(uint16_t index);
    size_t get_message_size(uint16_t index);
    bool is_mesage(uint16_t index);
    bool message_available() { return (messages_.begin()->first == seq_num_); }
    uint16_t get_ack_num() { return ack_num_; }
    void set_ack_num(uint16_t ack_num) { ack_num_ = ack_num; }
    void set_seq_num(uint16_t seq_num) { seq_num_ = seq_num; }

private:
    uint16_t ack_num_;
    std::map<uint16_t, std::vector<uint8_t>> messages_;
};

} } // namespace eprosima::micrortps

#endif // XRCESTREAMS_H
