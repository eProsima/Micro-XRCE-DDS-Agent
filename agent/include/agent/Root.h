#ifndef ROOT_H
#define ROOT_H

#include <agent/ObjectVariant.h>
#include <agent/client/ProxyClient.h>
#include <agent/XRCEFactory.h>
#include <agent/XRCEParser.h>

#include <transport/ddsxrce_transport.h>

#include <map>

namespace eprosima{
namespace micrortps{

class Agent;
Agent& root();

class Agent : public XRCEListener
{
public:
    Agent();
    ~Agent() = default;

    void init();

    Status create_client(int32_t client_key,  const ObjectVariant& client_representation);
    Status delete_client(int32_t client_key);

    void run();

    void on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const CREATE_PAYLOAD&      create_payload) override;
    void on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const DELETE_PAYLOAD&      create_payload) override;
    void on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const WRITE_DATA_PAYLOAD&  write_payload)  override;
    void on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const READ_DATA_PAYLOAD&   read_payload)   override;

private:
    channel_id_t ch_id;
    static const size_t buffer_len = 1024;
    octet out_buffer[buffer_len];
    octet in_buffer[buffer_len];
    locator_t loc;

    std::map<int32_t, ProxyClient> clients_;
};

} // eprosima
} // micrortps

#endif //_ROOT_H