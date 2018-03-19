#ifndef XRCESTREAMS_H
#define XRCESTREAMS_H

#include <vector>
#include <map>

namespace eprosima { namespace micrortps {

class XRCEStream
{
public:
    XRCEStream() : next_seq_num_(0) {}

    uint16_t get_next_seq_num() { return next_seq_num_; }
    virtual void update_seq_num(uint16_t seq_num) = 0;

protected:
    uint16_t next_seq_num_;
};

class BestEffortStream : public XRCEStream
{
public:
    BestEffortStream() : XRCEStream() {}

    void update_seq_num(uint16_t seq_num) override;
};

class ReliableStream : public XRCEStream
{
public:
    ReliableStream() : XRCEStream(), nack_bitmap_(0), messages_() {}

    void update_seq_num(uint16_t seq_num) override;
    void insert_message(uint16_t seq_num, const char* buf, size_t len);

private:
    uint16_t nack_bitmap_;
    std::map<uint8_t, std::vector<uint8_t>> messages_;
};

} } // namespace eprosima::micrortps

#endif // XRCESTREAMS_H
