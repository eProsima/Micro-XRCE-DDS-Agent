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

#ifndef UXR_AGENT_TRANSPORT_MULTISERIAL_SERIALAGENTLINUX_HPP_
#define UXR_AGENT_TRANSPORT_MULTISERIAL_SERIALAGENTLINUX_HPP_

#include <uxr/agent/transport/Server.hpp>
#include <uxr/agent/transport/endpoint/MultiSerialEndPoint.hpp>
#include <uxr/agent/transport/stream_framing/StreamFramingProtocol.hpp>
#include <uxr/agent/utils/SharedMutexPriority.hpp>

#include <cstdint>
#include <cstddef>
#include <sys/poll.h>

#include <mutex>
#include <condition_variable>

namespace eprosima {
namespace uxr {

class MultiSerialAgent : public Server<MultiSerialEndPoint>
{
public:
    MultiSerialAgent(
            uint8_t addr,
            Middleware::Kind middleware_kind);
    
    void insert_serial(int serial_fd);
    bool remove_serial(int serial_fd);

#ifdef UAGENT_DISCOVERY_PROFILE
    bool has_discovery() final { return false; }
#endif

#ifdef UAGENT_P2P_PROFILE
    bool has_p2p() final { return false; }
#endif

private:
    virtual bool init() = 0;

    virtual bool fini() = 0;

    bool recv_message(
            InputPacket<MultiSerialEndPoint>& /* input_packet */,
            int /* timeout */,
            TransportRc& /* transport_rc */) final { 
                    return false; 
                };

    bool recv_message(
            std::vector<InputPacket<MultiSerialEndPoint>>& input_packet,
            int timeout,
            TransportRc& transport_rc) final;

    bool send_message(
            OutputPacket<MultiSerialEndPoint> output_packet,
            TransportRc& transport_rc) final;

    ssize_t write_data(
            uint8_t serial_fd,
            uint8_t* buf,
            size_t len,
            TransportRc& transport_rc);

    ssize_t read_data(
            uint8_t serial_fd,
            uint8_t* buf,
            size_t len,
            int timeout,
            TransportRc& transport_rc);

protected:
    std::mutex error_mtx;
    std::vector<int> error_fd;
    
    utils::SharedMutexPriority framing_mtx;
    std::map<int, FramingIO> framing_io;
    fd_set read_fds;

    uint8_t addr_;
    uint8_t buffer_[SERVER_BUFFER_SIZE];
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_TRANSPORT_MULTISERIAL_SERIALAGENTLINUX_HPP_
