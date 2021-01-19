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

#include <cstdint>
#include <cstddef>

namespace eprosima {
namespace uxr {

/**
 * @brief This class allows final users to easily implement a custom
 *        Micro XRCE-DDS Agent, in terms of transport behaviour.
 *        To do so, several methods must be implemented, in order to init
 *        an agent instance, close it, and send and receive messages using
 *        the desired transport mechanism, as well as handling possible errors.
 */
template<typename EndPoint>
class CustomAgent : public Server<EndPoint>
{
public:

    /**
     * @brief Method signatures that users must implement to create a custom agent.
     *        They include the same input and return parameters as in the Server
     *        class' virtual methods, plus a pointer to the self CustomAgent instance
     *        which will store them. Also, for read and write operations, a pointer
     *        to the octet buffer is provided, to use it when required.
     */
    using InitFunction = std::function<bool (
        CustomAgent<EndPoint> *)>;
    using FiniFunction = std::function<bool (
        CustomAgent<EndPoint> *)>;
    using RecvMsgFunction = std::function<bool (
        CustomAgent<EndPoint> *,
        uint8_t *,
        InputPacket<EndPoint> &,
        int,
        TransportRc &)>;
    using SendMsgFunction = std::function<bool (
        CustomAgent<EndPoint> *,
        uint8_t *,
        OutputPacket<EndPoint>,
        TransportRc &)>;
    using HandleErrorFunction = std::function<bool (
        CustomAgent<EndPoint> *,
        TransportRc)>;

    /**
     * @brief Constructor.
     * @param middleware_kind   The middleware selected to represent the XRCE entities
     *                          in the DDS world (FastDDS, FastRTPS, CED...)
     * @param init_function     Custom user-defined function, called during initialization.
     * @param fini_function     Custom user-defined function, called upon agent's destruction.
     * @param recv_msg_function Custom user-defined function, called when receiving some data.
     * @param send_msg_function Custom user-defined function, called when sending some information.
     * @param error_function    Custom user-defined function, called to recover from an error.
     */
    CustomAgent(
            Middleware::Kind middleware_kind,
            InitFunction&& init_function,
            FiniFunction&& fini_function,
            RecvMsgFunction&& recv_msg_function,
            SendMsgFunction&& send_msg_function,
            HandleErrorFunction&& error_function)
        : Server<EndPoint>(middleware_kind)
        , custom_init_func_(std::move(init_function))
        , custom_fini_func_(std::move(fini_function))
        , custom_recv_msg_func_(std::move(recv_msg_function))
        , custom_send_msg_func_(std::move(send_msg_function))
        , custom_handle_error_func_(std::move(error_function))
    {
    }

    /**
     * @brief Destructor.
     */
    ~CustomAgent() final
    {
        try
        {
            this->stop();
        }
        catch(std::exception& e)
        {
            UXR_AGENT_LOG_CRITICAL(
                UXR_DECORATE_RED("Error stopping custom agent server"),
                "exception: {}",
                e.what());
        }
    }

    /**
     * @brief Helper method to retrieve the custom agent's internal buffer size.
     *        Useful for some read/write operations.
     * @return Buffer size, in bytes.
     */
    uint16_t buffer_size() const
    {
        return static_cast<uint16_t>(SERVER_BUFFER_SIZE);
    }

private:
    /**
     * @brief Override virtual Server operations.
     */
    inline bool init() final
    {
        return custom_init_func_(this);
    }

    inline bool fini() final
    {
        return custom_fini_func_(this);
    }

#ifdef UAGENT_DISCOVERY_PROFILE
    inline bool init_discovery(
            uint16_t /*discovery_port*/) final
    {
        return false;
    }

    inline bool fini_discovery() final
    {
        return false;
    }
#endif // UAGENT_DISCOVERY_PROFILE

#ifdef UAGENT_P2P_PROFILE
    inline bool init_p2p(
            uint16_t /*p2p_port*/) final
    {
        return false;
    }

    inline bool fini_p2p() final
    {
        return false;
    }
#endif // UAGENT_P2P_PROFILE

    inline bool recv_message(
            InputPacket<EndPoint>& input_packet,
            int timeout,
            TransportRc& transport_rc) final
    {
        return custom_recv_msg_func_(this, buffer_, input_packet, timeout, transport_rc);
    }

    inline bool send_message(
            OutputPacket<EndPoint> output_packet,
            TransportRc& transport_rc) final
    {
        return custom_send_msg_func_(this, buffer_, output_packet, transport_rc);
    }

    inline bool handle_error(
            TransportRc transport_rc) final
    {
        return custom_handle_error_func_(this, transport_rc) && fini() && init();
    }

    uint8_t buffer_[SERVER_BUFFER_SIZE];

    /**
     * @brief Store user-defined operations for the custom agent server.
     */
    InitFunction custom_init_func_;

    FiniFunction custom_fini_func_;

    RecvMsgFunction custom_recv_msg_func_;

    SendMsgFunction custom_send_msg_func_;

    HandleErrorFunction custom_handle_error_func_;
};
} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_TRANSPORT_CUSTOM_AGENT_HPP_