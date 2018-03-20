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

    bool is_next_message(dds::xrce::StreamId stream_id, uint16_t seq_num);
    bool is_valid_message(dds::xrce::StreamId stream_id, uint16_t seq_num);
    void promote_seq_num(dds::xrce::StreamId stream_id, uint16_t seq_num);
    void update_from_heartbeat(dds::xrce::StreamId stream_id, uint16_t first_unacked, uint16_t last_unacked);
    bool message_available(dds::xrce::StreamId stream_id);
    dds::xrce::XrceMessage get_next_message(dds::xrce::StreamId stream_id);
    void store_message(dds::xrce::StreamId stream_id, uint16_t seq_num, const char* buf, size_t len);

private:
    std::map<dds::xrce::StreamId, BestEffortStream> input_best_effort_streams_;
    std::map<dds::xrce::StreamId, ReliableStream> input_relible_streams_;
    std::map<dds::xrce::StreamId, BestEffortStream> output_best_effort_streams_;
    std::map<dds::xrce::StreamId, ReliableStream> output_relible_streams_;
};

} } // namespace eprosima::micrortps

#endif // STREAMSMANAGER_H
