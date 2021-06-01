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

#ifndef UXR_AGENT_TRANSPORT_UDPv6_AGENT_HPP_
#define UXR_AGENT_TRANSPORT_UDPv6_AGENT_HPP_

#include <uxr/agent/transport/Server.hpp>
#include <uxr/agent/transport/endpoint/IPv6EndPoint.hpp>
#ifdef UAGENT_DISCOVERY_PROFILE
#include <uxr/agent/transport/discovery/DiscoveryServerWindows.hpp>
#endif

#include <winsock2.h>
#include <cstdint>
#include <cstddef>

namespace eprosima {
namespace uxr {

extern template class Server<IPv6EndPoint>; // Explicit instantiation declaration.

class UDPv6Agent : public Server<IPv6EndPoint>
{
public:
    UXR_AGENT_EXPORT UDPv6Agent(
            uint16_t agent_port,
            Middleware::Kind middleware_kind);

    UXR_AGENT_EXPORT ~UDPv6Agent() final;

#ifdef UAGENT_DISCOVERY_PROFILE
    bool has_discovery() final { return true; }
#endif

#ifdef UAGENT_P2P_PROFILE
    bool has_p2p() final { return false; }
#endif

private:
    bool init() final;

    bool fini() final;

#ifdef UAGENT_DISCOVERY_PROFILE
    bool init_discovery(uint16_t discovery_port) final;

    bool fini_discovery() final;
#endif

    bool recv_message(
            InputPacket<IPv6EndPoint>& input_packet,
            int timeout,
            TransportRc& transport_rc) final;

    bool send_message(
            OutputPacket<IPv6EndPoint> output_packet,
            TransportRc& transport_rc) final;

    bool handle_error(
            TransportRc transport_rc) final;

private:
    WSAPOLLFD poll_fd_;
    uint8_t buffer_[SERVER_BUFFER_SIZE];
    uint16_t agent_port_;
#ifdef UAGENT_DISCOVERY_PROFILE
    DiscoveryServerWindows<IPv6EndPoint> discovery_server_;
#endif
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_TRANSPORT_UDPv6_AGENT_HPP_
