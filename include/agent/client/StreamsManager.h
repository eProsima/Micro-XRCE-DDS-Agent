#ifndef STREAMSMANAGER_H
#define STREAMSMANAGER_H

#include <agent/client/XRCEStreams.h>
#include <XRCETypes.h>
#include <MessageHeader.h>

#include <memory>
#include <map>

namespace eprosima { namespace micrortps {

class StreamsManager
{
public:
    StreamsManager();

    /* Input streams functions. */
    bool is_next_message(dds::xrce::StreamId stream_id, uint16_t seq_num);
    bool is_valid_message(dds::xrce::StreamId stream_id, uint16_t seq_num);
    void update_from_heartbeat(dds::xrce::StreamId stream_id, uint16_t first_unacked, uint16_t last_unacked);
    void promote_seq_num(dds::xrce::StreamId stream_id, uint16_t seq_num);
    bool message_available(dds::xrce::StreamId stream_id);
    dds::xrce::XrceMessage get_next_message(dds::xrce::StreamId stream_id);
    void store_input_message(dds::xrce::StreamId stream_id, uint16_t seq_num, const char* buf, size_t len);
    uint16_t get_first_unacked_seq_num(dds::xrce::StreamId stream_id);

    /* Output streams functions. */
    uint16_t get_ack_num(dds::xrce::StreamId stream_id);
    void store_output_message(dds::xrce::StreamId stream_id, const char* buf, size_t len);
    dds::xrce::XrceMessage get_output_message(dds::xrce::StreamId stream_id, uint16_t index);
    std::array<uint8_t, 2> get_nack_bitmap(dds::xrce::StreamId stream_id);
    uint16_t get_first_unacked(dds::xrce::StreamId stream_id);
    uint16_t get_last_unacked(dds::xrce::StreamId stream_id);

private:
    std::map<dds::xrce::StreamId, BestEffortStream> input_best_effort_streams_;
    std::map<dds::xrce::StreamId, ReliableStream> input_relible_streams_;
    std::map<dds::xrce::StreamId, BestEffortStream> output_best_effort_streams_;
    std::map<dds::xrce::StreamId, ReliableStream> output_relible_streams_;
};

} } // namespace eprosima::micrortps

#endif // STREAMSMANAGER_H
