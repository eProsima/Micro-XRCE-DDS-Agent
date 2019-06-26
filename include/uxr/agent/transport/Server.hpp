// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
#include <uxr/agent/transport/SessionManager.hpp>
#include <uxr/agent/transport/endpoint/EndPoint.hpp>
#include <uxr/agent/scheduler/FCFSScheduler.hpp>
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

    UXR_AGENT_EXPORT bool run();
    UXR_AGENT_EXPORT bool stop();

#ifdef UAGENT_DISCOVERY_PROFILE
    UXR_AGENT_EXPORT bool enable_discovery(uint16_t discovery_port = DISCOVERY_PORT);
    UXR_AGENT_EXPORT bool disable_discovery();
#endif

#ifdef UAGENT_P2P_PROFILE
    UXR_AGENT_EXPORT bool enable_p2p(uint16_t p2p_port);
    UXR_AGENT_EXPORT bool disable_p2p();
#endif

private:
    void push_output_packet(OutputPacket output_packet);

//    virtual void on_create_client(
//            EndPoint* source,
//            const dds::xrce::CLIENT_Representation& representation) = 0;
//
//    virtual void on_delete_client(EndPoint* source) = 0;
//
//    virtual const dds::xrce::ClientKey get_client_key(EndPoint* source) = 0;
//
//    virtual std::unique_ptr<EndPoint> get_source(const dds::xrce::ClientKey& client_key) = 0;

    virtual bool init() = 0;

    virtual bool close() = 0;

#ifdef UAGENT_DISCOVERY_PROFILE
    virtual bool init_discovery(uint16_t discovery_port) = 0;

    virtual bool close_discovery() = 0;
#endif

#ifdef UAGENT_P2P_PROFILE
    virtual bool init_p2p(uint16_t p2p_port) = 0;

    virtual bool close_p2p() = 0;
#endif

    virtual bool recv_message(
            InputPacket& input_packet,
            int timeout) = 0;

    virtual bool send_message(OutputPacket output_packet) = 0;

    virtual int get_error() = 0;

    void receiver_loop();

    void sender_loop();

    void processing_loop();

    void heartbeat_loop();

protected:
    Processor<EndPoint>* processor_;

private:
    std::mutex mtx_;
    std::thread receiver_thread_;
    std::thread sender_thread_;
    std::thread processing_thread_;
    std::thread heartbeat_thread_;
    std::atomic<bool> running_cond_;
    FCFSScheduler<InputPacket> input_scheduler_;
    FCFSScheduler<OutputPacket> output_scheduler_;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_TRANSPORT_SERVER_HPP_
