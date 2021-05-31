// Copyright 2021-present Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef UXR_AGENT_TRANSPORT_CUSTOM_AGENT_HPP_
#define UXR_AGENT_TRANSPORT_CUSTOM_AGENT_HPP_

#include <uxr/agent/transport/Server.hpp>
#include <uxr/agent/transport/endpoint/CustomEndPoint.hpp>
#include <uxr/agent/transport/stream_framing/StreamFramingProtocol.hpp>

#include <cstdint>
#include <cstddef>

#ifdef _WIN32
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

namespace eprosima {
namespace uxr {

/**
 * @brief This class allows final users to easily implement a custom
 *        Micro XRCE-DDS Agent, in terms of transport behaviour.
 *        To do so, several methods must be implemented, in order to init
 *        an agent instance, close it, and send and receive messages using
 *        the desired transport mechanism.
 */

class CustomAgent : public Server<CustomEndPoint>
{
public:
    /**
     * @brief Init function signature, to be implemented by final users.
     * return true if successful transport initialization; false otherwise.
     */
    using InitFunction = std::function<bool ()>;

    /**
     * @brief Fini function signature, to be implemented by final users.
     * return true if successful transport shutdown; false otherwise.
     */
    using FiniFunction = std::function<bool ()>;

    /**
     * @brief Receive message function signature, to be implemented by final users.
     * @param source_endpoint User-defined, it should be filled accordingly
     *        with the source metadata acquired from receiving a new packet/
     *        byte stream through the middleware.
     * @param buffer Pointer to octec buffer used to receive the information.
     * @param buffer_length Reception buffer size.
     * @param timeout Connection timeout for receiving a new message.
     * @param transport_rc Transport return code, to be filled by the user.
     * @return ssize_t Number of received bytes.
     */
    using RecvMsgFunction = std::function<ssize_t (
        CustomEndPoint* /*source_endpoint*/,
        uint8_t* /*buffer*/,
        size_t /*buffer_length*/,
        int /*timeout*/,
        TransportRc& /*transport_rc*/)>;
    /**
     * @brief Send message function signature, to be implemented by final users.
     * @param destination_endpoint Allows to retrieve the required endpoint
     *        information to send the message back to the client.
     * @param buffer Holds the message to be sent back to the client.
     * @param message_length Number of bytes to be sent.
     * @param transport_rc Transport return code, to be filled by the user.
     * @return ssize_t Number of sent bytes.
     */
    using SendMsgFunction = std::function<ssize_t (
        const CustomEndPoint* /*destination_endpoint*/,
        uint8_t* /*buffer*/,
        size_t /*message_length*/,
        TransportRc& /*transport_rc*/)>;

    /**
     * @brief Constructor.
     * @param name Name of the middleware to be implemented by this CustomAgent.
     * @param middleware_kind The middleware selected to represent the XRCE entities
     *        in the DDS world (FastDDS, FastRTPS, CED...)
     * @param framing Whether this agent transport shall use framing or not.
     * @param init_function Custom user-defined function, called during initialization.
     * @param fini_function Custom user-defined function, called upon agent's destruction.
     * @param recv_msg_function Custom user-defined function, called when receiving some data.
     * @param send_msg_function Custom user-defined function, called when sending some information.
     */
    UXR_AGENT_EXPORT CustomAgent(
            const std::string& name,
            CustomEndPoint* endpoint,
            Middleware::Kind middleware_kind,
            bool framing,
            InitFunction& init_function,
            FiniFunction& fini_function,
            SendMsgFunction& send_msg_function,
            RecvMsgFunction& recv_msg_function);

    /**
     * @brief Destructor.
     */
    UXR_AGENT_EXPORT ~CustomAgent() final;

private:
    /**
     * @brief Override virtual Server operations.
     */
    bool init() final;

    bool fini() final;

#ifdef UAGENT_DISCOVERY_PROFILE
    bool has_discovery() final { return false; }
#endif // UAGENT_DISCOVERY_PROFILE

#ifdef UAGENT_P2P_PROFILE
    bool has_p2p() final { return false; }
#endif // UAGENT_P2P_PROFILE

    bool recv_message(
            InputPacket<CustomEndPoint>& input_packet,
            int timeout,
            TransportRc& transport_rc) final;

    bool send_message(
            OutputPacket<CustomEndPoint> output_packet,
            TransportRc& transport_rc) final;

    bool handle_error(
            TransportRc transport_rc) final;

    /**
     * @brief Internal buffer used for receiving messages.
     */
    uint8_t buffer_[SERVER_BUFFER_SIZE];

    /**
     * @brief Custom agent middleware's name.
     */
    const std::string name_;

    /**
     * @brief Pointers to this custom agent's endpoint definition.
     *        They are used for receive and send operations, respectively.
     */
    CustomEndPoint* recv_endpoint_;
    CustomEndPoint* send_endpoint_;

    /**
     * @brief Reference to user-defined operations for the custom agent server.
     */
    InitFunction& custom_init_func_;
    FiniFunction& custom_fini_func_;
    SendMsgFunction& custom_send_msg_func_;
    RecvMsgFunction& custom_recv_msg_func_;

    /**
     * @brief Indicates the usage or non-usage of framing for R/W operations.
     */
    bool framing_;

    /**
     * @brief Holds the framing logics, if framing is used.
     */
    FramingIO framing_io_;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_TRANSPORT_CUSTOM_AGENT_HPP_