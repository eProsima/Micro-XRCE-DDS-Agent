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

#ifndef UXR_AGENT_TRANSPORT_DISCOVERY_SERVER_HPP_
#define UXR_AGENT_TRANSPORT_DISCOVERY_SERVER_HPP_

#include <uxr/agent/message/Packet.hpp>

#include <thread>
#include <atomic>
#include <mutex>

namespace eprosima {
namespace uxr {

class Processor;

class DiscoveryServer
{
public:
    DiscoveryServer(const Processor& processor);

    virtual ~DiscoveryServer() = default;

    bool run(
            uint16_t discovery_port,
            const dds::xrce::TransportAddress& transport_address_);

    bool stop();

    void set_filter_port(uint16_t filter_port) { filter_port_ = filter_port; }

private:
    virtual bool init(uint16_t discovery_port) = 0;

    virtual bool close() = 0;

    virtual bool recv_message(
            InputPacket& input_packet,
            int timeout) = 0;

    virtual bool send_message(OutputPacket&& output_packet) = 0;

    void discovery_loop();

private:
    std::mutex mtx_;
    std::thread thread_;
    std::atomic<bool> running_cond_;
    const Processor& processor_;

protected:
    dds::xrce::TransportAddress transport_address_;
    uint16_t filter_port_;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_TRANSPORT_DISCOVERY_SERVER_HPP_
