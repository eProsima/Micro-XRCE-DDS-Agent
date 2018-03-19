// Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _ROOT_H
#define _ROOT_H

#include <agent/client/ProxyClient.h>
#include <agent/XRCEFactory.h>
#include <agent/XRCEParser.h>
#include "MessageQueue.h"
#include <MessageHeader.h>
#include <SubMessageHeader.h>

#include <micrortps/transport/micrortps_transport.h>

#include <thread>
#include <memory>
#include <map>

namespace eprosima{
namespace micrortps{

class Agent;

Agent* root();

/*!
 * \brief The Agent class handle XRCE messages and distribute them to different ProxyClients.
 * 		  It implement the XRCEListener interface for receive messages from a XRCEParser.
 */
class Agent : public XRCEListener
{
public:
    Agent();
    ~Agent() = default;

    /*!
     * \brief Initializes the Agent using serial communication.
     * \param  device Name of the device, for example, in Linux one could be "/dev/ttyACM0".
     */
    void init(const std::string& device);

    /*!
     * \brief Initializes the Agent using UDP communication.
     * \param local_port    The local port.
     */
    void init(const uint16_t local_port);

    /*!
     * \brief Creates and stores a ProxyClient in a ClientKey map.
     * \param header		The Incoming message header.
     * \param create_info	The CLIENT_CREATE submessage payload.
     * \return If create_info's xrce_cookie does not match XRCE_COOKIE return {STATUS_LAST_OP_CREATE, STATUS_ERR_INVALID_DATA}.
     * \return If relase mayor version (xrce_version[0]) does not match XRCE_VERSION_MAJOR return {STATUS_LAST_OP_CREATE, STATUS_ERR_INCOMPATIBLE}.
     * \return In other cases return {STATUS_LAST_OP_CREATE, STATUS_OK}.
     */
//    dds::xrce::ResultStatus create_client(const dds::xrce::MessageHeader& header, const dds::xrce::CREATE_CLIENT_Payload& create_info);
    dds::xrce::ResultStatus create_client(const dds::xrce::CREATE_CLIENT_Payload& payload);

    /*!
     * \brief Removes a previously stored ProxyClient.
     * \param client_key	Client's key.
     * \param delete_info	DELETE submessage payload.
     * \return If the object is not found return {STATUS_LAST_OP_DELETE, STATUS_ERR_INVALID_DATA}.
     * \return In other cases return {STATUS_LAST_OP_DELETE, STATUS_OK}.
     */
    dds::xrce::ResultStatus delete_client(dds::xrce::ClientKey client_key);

    /*!
     * \brief Starts a event loop in order to receive messages from Clients.
     */
    void run();

    /*!
     * \brief Stops the messages receiver event loop.
     */
    void stop();

    /*!
     * \brief Receiver of CREATE_CLIENT submessages.
     * \param header 				The message header.
     * \param sub_header 			The submessage header.
     * \param create_client_payload	The submessage payload.
     */
    void on_message(const dds::xrce::MessageHeader& header,
                    const dds::xrce::SubmessageHeader&,
                    const dds::xrce::CREATE_CLIENT_Payload& create_client_payload) override;

    /*!
     * \brief Receiver of CREATE submessages.
     * \param header			The message header.
     * \param sub_header		The submessage header.
     * \param create_payload	The submessage payload.
     */
    void on_message(const dds::xrce::MessageHeader& header,
                    const dds::xrce::SubmessageHeader& sub_header,
                    const dds::xrce::CREATE_Payload& create_payload) override;

    /*!
     * \brief Receiver of DELETE submessages.
     * \param header			The message header.
     * \param sub_header		The submessage header.
     * \param delete_payload	The submessage payload.
     */
    void on_message(const dds::xrce::MessageHeader& header,
                    const dds::xrce::SubmessageHeader& sub_header,
                    const dds::xrce::DELETE_Payload& delete_payload) override;

    /*!
     * \brief Receiver of WRITE_DATA submessages.
     * \param header		The message header.
     * \param sub_header	The submessage header.
     * \param write_payload The submessage payload.
     */
    void on_message(const dds::xrce::MessageHeader& header,
                    const dds::xrce::SubmessageHeader& sub_header,
                    dds::xrce::WRITE_DATA_Payload_Data& write_payload)  override;

    /*!
     * \brief Receiver of READ_DATA submessage.
     * \param header		The message header.
     * \param sub_header	The submessage header.
     * \param read_payload 	The submessage payload.
     */
    void on_message(const dds::xrce::MessageHeader& header,
                    const dds::xrce::SubmessageHeader& sub_header,
                    const dds::xrce::READ_DATA_Payload& read_payload)   override;

    /*!
     * \brief Function call when a ACKNACK message arrives.
     * \param header 			The message header.
     * \param sub_header 		The submessage header.
     * \param acknack_payload 	The submessage payload.
     */
    void on_message(const dds::xrce::MessageHeader &header,
                    const dds::xrce::SubmessageHeader &sub_header,
                    const dds::xrce::ACKNACK_Payload &acknack_payload) override;

    /*!
     * \brief Function call when a HEARTBEAT message arrives.
     * \param header 			The message header.
     * \param sub_header 		The submessage header.
     * \param heartbeat_payload	The submessage payload.
     */
    void on_message(const dds::xrce::MessageHeader &header,
                    const dds::xrce::SubmessageHeader &sub_header,
                    const dds::xrce::HEARTBEAT_Payload &heartbeat_payload) override;

    /*!
     * \brief Gets a Client based its key.
     * \param  The client's key.
     * \return If the Client does not exit return a nullptr.
     * \return In other cases return a pointer to the Client.
     */
    ProxyClient* get_client(dds::xrce::ClientKey client_key);

    /*!
     * \brief Pushs messages in a output queue. These messages are delivered to Clients in a event
     *        loop that is running in a separate thread. This methods launch the thread at the
     *        first output message.
     * \param message The output message.
     */
    void add_reply(const Message& message);

    /*!
     * \brief Adds STATUS submessages to the given message header and pushs it in the output queue.
     * \param header 	   The message header.
     * \param status_reply The STATUS submessage payload.
     */
    void add_reply(const dds::xrce::MessageHeader& header, const dds::xrce::STATUS_Payload& status_reply);

    /*!
     * \brief Adds DATA submessages with FORMAT_DATA format to the given message header and pushs it in the output queue.
     * \param header 	The message header.
     * \param payload 	The DATA submessage payload.
     */
    void add_reply(const dds::xrce::MessageHeader& header, const dds::xrce::DATA_Payload_Data& payload);

    /*!
     * \brief Adds DATA submessages with FORMAT_DATA format to the given message header and pushs it in the output queue.
     * \param header 	The message header.
     * \param payload 	The DATA submessage payload.
     */
    void add_reply(const dds::xrce::MessageHeader& header, const dds::xrce::DATA_Payload_Sample& payload);

    /*!
     * \brief Adds DATA submessages with FORMAT_DATA_SEQ format to the given message header and pushs it in the output queue.
     * \param header 	The message header.
     * \param payload 	The DATA submessage payload.
     */
    void add_reply(const dds::xrce::MessageHeader& header, const dds::xrce::DATA_Payload_DataSeq& payload);

    /*!
     * \brief Adds DATA submessages with FORMAT_SAMPLE_SEQ format to the given message header and pushs it in the output queue.
     * \param header 	The message header.
     * \param payload 	The DATA submessage payload.
     */
    void add_reply(const dds::xrce::MessageHeader& header, const dds::xrce::DATA_Payload_SampleSeq& payload);

    /*!
     * \brief Adds DATA submessages with FORMAT_PACKED_SAMPLES format to the given message header and pushs it in the output queue.
     * \param header 	The message header.
     * \param payload 	The DATA submessage payload.
     */
    void add_reply(const dds::xrce::MessageHeader& header, const dds::xrce::DATA_Payload_PackedSamples& payload);

    void add_reply(const dds::xrce::MessageHeader& header, const dds::xrce::ACKNACK_Payload& payload);

    void abort_execution();

private:
    /* Message processing functions. */
    void handle_input_message(const dds::xrce::XrceMessage& input_message);
    void process_message(const dds::xrce::MessageHeader& header, Serializer& deserializer, ProxyClient& client);

    /* Submessage procession functions. */
    void process_create_client(const dds::xrce::MessageHeader& header, Serializer& deserializer);
    void process_create(const dds::xrce::MessageHeader& header,
                        const dds::xrce::SubmessageHeader& sub_header,
                        Serializer& deserializer, ProxyClient& client);
    void process_delete(const dds::xrce::MessageHeader& header,
                        const dds::xrce::SubmessageHeader& sub_header,
                        Serializer& deserializer, ProxyClient& client);
    void process_write_data(const dds::xrce::MessageHeader& header,
                            const dds::xrce::SubmessageHeader& sub_header,
                            Serializer& deserializer, ProxyClient& client);
    void process_read_data(const dds::xrce::MessageHeader& header,
                           const dds::xrce::SubmessageHeader& sub_header,
                           Serializer& deserializer, ProxyClient& client);
    void process_acknack(const dds::xrce::MessageHeader& header,
                         const dds::xrce::SubmessageHeader& sub_header,
                         Serializer& deserializer, ProxyClient& client);
    void process_heartbeat(const dds::xrce::MessageHeader& header,
                           const dds::xrce::SubmessageHeader& sub_header,
                           Serializer& deserializer, ProxyClient& client);

private:

    micrortps_locator_t locator_;
    static const size_t buffer_len_ = 1024;
    octet out_buffer_[buffer_len_];
    octet in_buffer_[buffer_len_];
    std::mutex clientsmtx_;
    std::map<dds::xrce::ClientKey, ProxyClient> clients_;

    std::unique_ptr<std::thread> response_thread_;
    std::atomic<bool> running_;
    std::atomic<bool> reply_cond_;

    MessageQueue messages_;

    void reply();

    void demo_create_client();
    void demo_delete_subscriber(char* test_buffer, size_t buffer_size);
    void demo_message_create(char* test_buffer, size_t buffer_size);
    void demo_message_read(char* test_buffer, size_t buffer_size);
    void demo_process_response(Message& message);
    void demo_message_subscriber(char* test_buffer, size_t buffer_size);
    void demo_message_publisher(char* test_buffer, size_t buffer_size);
    void demo_message_write(char * test_buffer, size_t buffer_size);


    void update_header(dds::xrce::MessageHeader& header);
};

} // eprosima
} // micrortps

#endif //_ROOT_H
