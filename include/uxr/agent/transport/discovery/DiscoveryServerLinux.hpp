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

#ifndef _UXR_AGENT_TRANSPORT_DISCOVERY_SERVER_LINUX_HPP_
#define _UXR_AGENT_TRANSPORT_DISCOVERY_SERVER_LINUX_HPP_

#define UXR_DEFAULT_DISCOVERY_PORT 7400

#include <uxr/agent/transport/discovery/DiscoveryServer.hpp>
#include <uxr/agent/message/Packet.hpp>

#include <thread>
#include <atomic>
#include <sys/poll.h>

namespace eprosima {
namespace uxr {

class Processor;

class DiscoveryServerLinux : public DiscoveryServer
{
public:
    DiscoveryServerLinux(const Processor& processor, uint16_t port, uint16_t discovery_port);
    ~DiscoveryServerLinux() override = default;

private:
    bool init() override;
    bool close() override;
    bool recv_message(InputPacket& input_packet, int timeout) override;
    bool send_message(OutputPacket&& output_packet) override;

private:
    struct pollfd poll_fd_;
    uint8_t buffer_[128];
    uint16_t discovery_port_;
};

} // namespace uxr
} // namespace eprosima

#endif //_UXR_AGENT_TRANSPORT_DISCOVERY_SERVER_LINUX_HPP_
