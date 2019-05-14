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

#ifndef UXR_AGENT_TRANSPORT_SERIAL_SERVER_HPP_
#define UXR_AGENT_TRANSPORT_SERIAL_SERVER_HPP_

#include <uxr/agent/transport/serial/SerialServerBase.hpp>

#include <cstdint>
#include <cstddef>
#include <sys/poll.h>

namespace eprosima {
namespace uxr {

class SerialServer : public SerialServerBase
{
public:
    SerialServer(
            int fd,
            uint8_t addr,
            Middleware::Kind middleware_kind);

    ~SerialServer() = default;

private:
    bool init() final;

    bool close() final;

#ifdef PROFILE_DISCOVERY
    bool init_discovery(uint16_t /*discovery_port*/) final { return false; }

    bool close_discovery() final { return false; }
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

    static size_t write_data(
            void* instance,
            uint8_t* buf,
            size_t len);

    static size_t read_data(
            void* instance,
            uint8_t* buf,
            size_t len,
            int timeout);

private:
    struct pollfd poll_fd_;
    uint8_t buffer_[UINT16_MAX];
    uxrSerialIO serial_io_;
    int errno_;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_TRANSPORT_SERIAL_SERVER_HPP_
