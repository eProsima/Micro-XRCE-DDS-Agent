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

#ifndef UXR_AGENT_TRANSPORT_DISCOVERY_DISCOVERYSERVERWINDOWS_HPP_
#define UXR_AGENT_TRANSPORT_DISCOVERY_DISCOVERYSERVERWINDOWS_HPP_

#include <uxr/agent/transport/discovery/DiscoveryServer.hpp>
#include <uxr/agent/message/Packet.hpp>

#include <winsock2.h>
#include <thread>
#include <atomic>
#include <type_traits>

namespace eprosima {
namespace uxr {

template<typename EndPoint>
class DiscoveryServerWindows : public DiscoveryServer<EndPoint>
{
public:
    DiscoveryServerWindows(
            const Processor<EndPoint>& processor);

    ~DiscoveryServerWindows() override = default;

private:
    bool init(
            uint16_t discovery_port) final;

    bool close() final;

    bool recv_message(
            InputPacket<IPv4EndPoint>& input_packet,
            int timeout) final;

    bool send_message(
            OutputPacket<IPv4EndPoint>&& output_packet) final;

private:
    struct pollfd poll_fd_;
    uint8_t buffer_[128];
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_TRANSPORT_DISCOVERY_DISCOVERYSERVERWINDOWS_HPP_
