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

#ifndef UXR_AGENT_TRANSPORT_SERIAL_SERIALAGENTLINUX_HPP_
#define UXR_AGENT_TRANSPORT_SERIAL_SERIALAGENTLINUX_HPP_

#include <uxr/agent/transport/Server.hpp>
#include <uxr/agent/transport/endpoint/SerialEndPoint.hpp>
#include <uxr/agent/transport/serial/SerialProtocol.hpp>

#include <cstdint>
#include <cstddef>
#include <sys/poll.h>

namespace eprosima {
namespace uxr {

class SerialAgent : public Server<SerialEndPoint>
{
public:
    SerialAgent(
            uint8_t addr,
            Middleware::Kind middleware_kind);

private:
    virtual bool init() = 0;

    virtual bool fini() = 0;

#ifdef UAGENT_DISCOVERY_PROFILE
    bool init_discovery(
            uint16_t /*discovery_port*/) final { return false; }

    bool fini_discovery() final { return false; }
#endif

#ifdef UAGENT_P2P_PROFILE
    bool init_p2p(
            uint16_t /*p2p_port*/) final { return false; } // TODO

    bool fini_p2p() final { return false; } // TODO
#endif

    bool recv_message(
            InputPacket<SerialEndPoint>& input_packet,
            int timeout,
            TransportRc& transport_rc) final;

    bool send_message(
            OutputPacket<SerialEndPoint> output_packet,
            TransportRc& transport_rc) final;

    size_t write_data(
            uint8_t* buf,
            size_t len,
            TransportRc& transport_rc);

    size_t read_data(
            uint8_t* buf,
            size_t len,
            int timeout,
            TransportRc& transport_rc);

protected:
    const uint8_t addr_;
    struct pollfd poll_fd_;
    uint8_t buffer_[UINT16_MAX];
    SerialIO serial_io_;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_TRANSPORT_SERIAL_SERIALAGENTLINUX_HPP_
