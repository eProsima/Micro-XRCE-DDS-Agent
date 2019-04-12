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

#ifndef _UXR_AGENT_TRANSPORT_UDP_SERVER_HPP_
#define _UXR_AGENT_TRANSPORT_UDP_SERVER_HPP_

#include <uxr/agent/transport/udp/UDPServerBase.hpp>
#include <uxr/agent/transport/udp/UDPEndPoint.hpp>
#include <uxr/agent/transport/discovery/DiscoveryServerWindows.hpp>

#include <winsock2.h>
#include <cstdint>
#include <cstddef>

namespace eprosima {
namespace uxr {

class UDPServer : public UDPServerBase
{
public:
    UXR_AGENT_LIB UDPServer(uint16_t port, uint16_t discovery_port = UXR_DEFAULT_DISCOVERY_PORT);
    UXR_AGENT_LIB ~UDPServer() = default;

private:
    bool init(bool discovery_enabled) final;
    bool close() final;
    bool recv_message(InputPacket& input_packet, int timeout) final;
    bool send_message(OutputPacket output_packet) final;
    int get_error() final;

private:
    WSAPOLLFD poll_fd_;
    uint8_t buffer_[UINT16_MAX];
    DiscoveryServerWindows discovery_server_;
};

} // namespace uxr
} // namespace eprosima

#endif //_UXR_AGENT_TRANSPORT_UDP_SERVER_HPP_
