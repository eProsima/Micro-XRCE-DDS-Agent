// Copyright 2017-present Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef UXR_AGENT_TRANSPORT_SERVER_HPP_
#define UXR_AGENT_TRANSPORT_SERVER_HPP_

#include <uxr/agent/Agent.hpp>
#include <uxr/agent/transport/TransportRc.hpp>
#include <uxr/agent/transport/SessionManager.hpp>
#include <uxr/agent/scheduler/PacketScheduler.hpp>
#include <uxr/agent/message/Packet.hpp>
#include <uxr/agent/processor/Processor.hpp>

#include <thread>

namespace eprosima {
namespace uxr {

template<typename EndPoint>
class Processor;

template<typename EndPoint>
class Server : public Agent, public SessionManager<EndPoint>
{
    friend class Processor<EndPoint>;
public:
    Server(Middleware::Kind middleware_kind);

    virtual ~Server();

    UXR_AGENT_EXPORT bool start();
    UXR_AGENT_EXPORT bool stop();

#ifdef UAGENT_DISCOVERY_PROFILE
    UXR_AGENT_EXPORT virtual bool has_discovery() = 0;
    UXR_AGENT_EXPORT bool enable_discovery(uint16_t discovery_port = DISCOVERY_PORT);
    UXR_AGENT_EXPORT bool disable_discovery();
#endif

#ifdef UAGENT_P2P_PROFILE
    UXR_AGENT_EXPORT virtual bool has_p2p() = 0;
    UXR_AGENT_EXPORT bool enable_p2p(uint16_t p2p_port);
    UXR_AGENT_EXPORT bool disable_p2p();
#endif

private:
    void push_output_packet(
            OutputPacket<EndPoint>&& output_packet);

    virtual bool init() = 0;

    virtual bool fini() = 0;

#ifdef UAGENT_DISCOVERY_PROFILE
    virtual bool init_discovery(uint16_t /* discovery_port */) {
                    return false;
                };

    virtual bool fini_discovery() {
                    return false;
                };
#endif

#ifdef UAGENT_P2P_PROFILE
    virtual bool init_p2p(uint16_t /* p2p_port */) {
                    return false;
                };

    virtual bool fini_p2p() {
                    return false;
                };
#endif

    virtual bool recv_message(
            InputPacket<EndPoint>& input_packet,
            int timeout,
            TransportRc& transport_rc) = 0;

    virtual bool recv_message(
            std::vector<InputPacket<EndPoint>>& /* input_packet */,
            int /* timeout */,
            TransportRc& /* transport_rc */) {
                    return false;
                };

    virtual bool send_message(
            OutputPacket<EndPoint> output_packet,
            TransportRc& transport_rc) = 0;

    virtual bool handle_error(TransportRc transport_rc) = 0;

    void receiver_loop();

    void sender_loop();

    void processing_loop();

    void heartbeat_loop();

    void error_handler_loop();

protected:
    Processor<EndPoint>* processor_;

private:
    std::mutex mtx_;
    std::thread receiver_thread_;
    std::thread sender_thread_;
    std::thread processing_thread_;
    std::thread heartbeat_thread_;
    std::thread error_handler_thread_;
    std::atomic<bool> running_cond_;
    PacketScheduler<InputPacket<EndPoint>> input_scheduler_;
    PacketScheduler<OutputPacket<EndPoint>> output_scheduler_;
    TransportRc transport_rc_;
    std::mutex error_mtx_;
    std::condition_variable error_cv_;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_TRANSPORT_SERVER_HPP_
