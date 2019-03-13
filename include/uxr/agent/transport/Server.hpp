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

#ifndef _UXR_AGENT_TRANSPORT_SERVER_HPP_
#define _UXR_AGENT_TRANSPORT_SERVER_HPP_

#include <uxr/agent/transport/EndPoint.hpp>
#include <uxr/agent/scheduler/FCFSScheduler.hpp>
#include <uxr/agent/message/Packet.hpp>
#include <uxr/agent/processor/Processor.hpp>
#include <uxr/agent/agent_dll.hpp>
#include <thread>

namespace eprosima {
namespace uxr {

class Processor;

class Server
{
public:
    Server();
    virtual ~Server();

    microxrcedds_agent_DllAPI bool run();
    microxrcedds_agent_DllAPI bool stop();
    microxrcedds_agent_DllAPI bool load_config_file(const std::string& path);

    void push_output_packet(OutputPacket output_packet);
    virtual void on_create_client(EndPoint* source, const dds::xrce::ClientKey& client_key) = 0;
    virtual void on_delete_client(EndPoint* source) = 0;
    virtual const dds::xrce::ClientKey get_client_key(EndPoint* source) = 0;
    virtual std::unique_ptr<EndPoint> get_source(const dds::xrce::ClientKey& client_key) = 0;

private:
    virtual bool init() = 0;
    virtual bool close() = 0;
    virtual bool recv_message(InputPacket& input_packet, int timeout) = 0;
    virtual bool send_message(OutputPacket output_packet) = 0;
    virtual int get_error() = 0;
    void receiver_loop();
    void sender_loop();
    void processing_loop();
    void heartbeat_loop();

protected:
    Processor* processor_;

private:
    std::unique_ptr<std::thread> receiver_thread_;
    std::unique_ptr<std::thread> sender_thread_;
    std::unique_ptr<std::thread> processing_thread_;
    std::unique_ptr<std::thread> heartbeat_thread_;
    std::atomic<bool> running_cond_;
    FCFSScheduler<InputPacket> input_scheduler_;
    FCFSScheduler<OutputPacket> output_scheduler_;
};

} // namespace uxr
} // namespace eprosima

#endif //_UXR_AGENT_TRANSPORT_SERVER_HPP_
