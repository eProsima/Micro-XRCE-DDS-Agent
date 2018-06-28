#ifndef STREAMSMANAGER_H
#define STREAMSMANAGER_H

#include <agent/client/XRCEStreams.h>

#include <memory>
#include <map>

namespace eprosima { namespace micrortps {

class StreamsManager
{
public:
    StreamsManager();

    /* Input streams functions. */
    bool is_next_message(dds::xrce::StreamId stream_id, SeqNum seq_num);
    bool is_valid_message(dds::xrce::StreamId stream_id, SeqNum seq_num);
    bool message_available(dds::xrce::StreamId stream_id);
    void update_from_heartbeat(dds::xrce::StreamId stream_id, const SeqNum first_unacked, const SeqNum last_unacked);
    void promote_stream(dds::xrce::StreamId stream_id, const SeqNum seq_num);
    XrceMessage get_next_message(dds::xrce::StreamId stream_id);
    void store_input_message(dds::xrce::StreamId stream_id, const SeqNum seq_num, const char* buf, size_t len);
    SeqNum get_first_unacked_seq_num(dds::xrce::StreamId stream_id);
    std::array<uint8_t, 2> get_nack_bitmap(dds::xrce::StreamId stream_id);

    /* Output streams functions. */
    void store_output_message(dds::xrce::StreamId stream_id, const char* buf, size_t len);
    XrceMessage get_output_message(dds::xrce::StreamId stream_id, const SeqNum index);
    SeqNum get_first_unacked_seq_nr(dds::xrce::StreamId stream_id);
    SeqNum get_last_unacked_seq_nr(dds::xrce::StreamId stream_id);
    void update_from_acknack(dds::xrce::StreamId stream_id, SeqNum first_unacked);
    SeqNum next_ouput_message(dds::xrce::StreamId stream_id);
    std::vector<uint8_t> get_output_streams();
    bool message_pending(dds::xrce::StreamId stream_id);

private:
    std::map<dds::xrce::StreamId, BestEffortStream> input_best_effort_streams_;
    std::map<dds::xrce::StreamId, ReliableInputStream> input_relible_streams_;
    std::map<dds::xrce::StreamId, BestEffortStream> output_best_effort_streams_;
    std::map<dds::xrce::StreamId, ReliableOutputStream> output_relible_streams_;
};

} } // namespace eprosima::micrortps

#endif // STREAMSMANAGER_H
