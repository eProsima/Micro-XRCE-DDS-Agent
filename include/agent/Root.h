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
#include <agent/MessageQueue.h>
#include <agent/MessageHeader.h>
#include <agent/SubMessageHeader.h>

#include <micrortps/transport/micrortps_transport.h>
#include <agent/transport/UDPServer.hpp>

#include <thread>
#include <memory>
#include <map>

#define HEARTBEAT_PERIOD 200
#define EPROSIMA_VENDOR_ID {0x01, 0x0F}

namespace eprosima{
namespace micrortps{

class Agent;

Agent& root();

/**
 * @brief The Agent class handle XRCE messages and distribute them to different ProxyClients.
 * 		  It implement the XRCEListener interface for receive messages from a XRCEParser.
 */
class Agent
{
public:
    Agent();
    ~Agent() = default;

    /**
     * @brief Initializes the Agent using serial communication.
     *
     * @param  device Name of the device, for example, in Linux one could be "/dev/ttyACM0".
     */
    bool init(const std::string& device);

    /**
     * @brief Initializes the Agent using UDP communication.
     *
     * @param local_port    The local port.
     */
    bool init(const uint16_t local_port);

    /**
     * @brief The XRCE Agent create a new ProxyClient with the specification of the client_representation.
     *
     * @param client_representation A representation of the Client.
     * @param agent_info            A representation of the Agent.
     * @param addr                  Client remote address.
     * @param port                  Client remote port.
     *
     * @return Indicates whether the operation suceeded and the current status of the XRCE.
     */
    dds::xrce::ResultStatus create_client(const dds::xrce::CLIENT_Representation& client_representation,
                                          dds::xrce::AGENT_Representation& agent_representation,
                                          uint32_t addr, uint16_t port);

    /**
     * @brief The XRCE Agent shall ckeck the ClientKey to locate an existing XRCE ProxyClient.
     *
     * @param client_key ProxyClient identifier.
     *
     * @return Indicates whether the operation succeeded and the current status of the object.
     */
    dds::xrce::ResultStatus delete_client(const dds::xrce::ClientKey& client_key);

    /**
     * @brief Starts a event loop in order to receive messages from Clients.
     */
    void run();

    /**
     * @brief Stops the messages receiver event loop.
     */
    void stop();

    /**
     * @brief Gets a Client based its key.
     *
     * @param  The client's key.
     * @return If the Client does not exit return a nullptr.
     *
     * @return In other cases return a pointer to the Client.
     */
    ProxyClient* get_client(const dds::xrce::ClientKey& client_key);

    /**
     * @brief Pushs messages in a output queue. These messages are delivered to Clients in a event
     *        loop that is running in a separate thread. This methods launch the thread at the
     *        first output message.
     *
     * @param message The output message.
     */
    void add_reply(Message& message);

    void abort_execution();

private:
    /* Send functions. */
    void reply();
    void manage_heartbeats();

    /* Utils functions. */
    ProxyClient* get_client(uint32_t addr);
    dds::xrce::ClientKey get_key(uint32_t addr);

    /* Message processing functions. */
    void handle_input_message(const XrceMessage& input_message, uint32_t addr, uint16_t port);
    void process_message(const dds::xrce::MessageHeader& header, Serializer& deserializer, ProxyClient& client);

    /* Submessage procession functions. */
    void process_create_client(const dds::xrce::MessageHeader& header,
                               Serializer& deserializer,
                               uint32_t addr, uint16_t port);
    void process_delete_client(const dds::xrce::MessageHeader& header,
                               Serializer& deserializer,
                               uint32_t addr, uint16_t port);
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
    UDPServer* udp_server_;
    micrortps_locator_t locator_;
    static const size_t buffer_len_ = CONFIG_MAX_TRANSMISSION_UNIT_SIZE;
    uint8_t input_buffer_[buffer_len_];
    std::mutex clientsmtx_;
    std::map<dds::xrce::ClientKey, ProxyClient> clients_;
    std::map<uint32_t, dds::xrce::ClientKey> addr_to_key_;

    std::unique_ptr<std::thread> response_thread_;
    std::unique_ptr<std::thread> heartbeats_thread_;
    std::atomic<bool> running_;
    std::atomic<bool> reply_cond_;
    std::atomic<bool> heartbeat_cond_;

    MessageQueue messages_;

};

} // eprosima
} // micrortps

#endif //_ROOT_H
