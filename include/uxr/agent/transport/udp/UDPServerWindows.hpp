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

#ifndef UXR_AGENT_TRANSPORT_UDP_SERVER_HPP_
#define UXR_AGENT_TRANSPORT_UDP_SERVER_HPP_

#include <uxr/agent/transport/udp/UDPServerBase.hpp>
#ifdef PROFILE_DISCOVERY
#include <uxr/agent/transport/discovery/DiscoveryServerWindows.hpp>
#endif

#include <winsock2.h>
#include <cstdint>
#include <cstddef>

namespace eprosima {
namespace uxr {

class UDPServer : public UDPServerBase
{
public:
    UXR_AGENT_EXPORT UDPServer(
            uint16_t agent_port,
            Middleware::Kind middleware_kind);

    UXR_AGENT_EXPORT ~UDPServer() final;

private:
    bool init() final;

    bool close() final;

#ifdef PROFILE_DISCOVERY
    bool init_discovery(uint16_t discovery_port) final;

    bool close_discovery() final;
#endif

#ifdef PROFILE_P2P
    bool init_p2p(uint16_t /*p2p_port*/) final { return false; } // TODO

    bool close_p2p() final { return false; } // TODO
#endif

    bool recv_message(
            InputPacket& input_packet,
            int timeout) final;

    bool send_message(OutputPacket output_packet) final;

    int get_error() final;

private:
    WSAPOLLFD poll_fd_;
    uint8_t buffer_[UINT16_MAX];
#ifdef PROFILE_DISCOVERY
    DiscoveryServerWindows discovery_server_;
#endif
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_TRANSPORT_UDP_SERVER_HPP_
