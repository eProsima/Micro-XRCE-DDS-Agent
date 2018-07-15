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

#ifndef _MICRORTPS_AGENT_TRANSPORT_UDP_SERVER_HPP_
#define _MICRORTPS_AGENT_TRANSPORT_UDP_SERVER_HPP_

#include <micrortps/agent/transport/Server.hpp>
#include <stdint.h>
#include <stddef.h>
#include <sys/poll.h>

namespace eprosima {
namespace micrortps {

/******************************************************************************
 * UDP EndPoint.
 ******************************************************************************/
class UDPEndPoint : public EndPoint
{
public:
    UDPEndPoint(uint32_t addr, uint8_t port) : addr_(addr), port_(port) {}
    ~UDPEndPoint() = default;

    uint32_t get_addr() const { return addr_; }
    uint16_t get_port() const { return port_; }

private:
    uint32_t addr_;
    uint16_t port_;
};

/******************************************************************************
 * UDP Server.
 ******************************************************************************/
class UDPServer : public Server
{
public:
    UDPServer(uint16_t port) : port_(port), poll_fd_{}, buffer_{0} {}
    ~UDPServer() = default;

    virtual bool recv_message(InputPacket& input_packet, int timeout) override;
    virtual bool send_message(OutputPacket output_packet) override;
    virtual int get_error() override;

private:
    virtual bool init() override;

private:   
    uint16_t port_;
    struct pollfd poll_fd_;
    uint8_t buffer_[1024];
};

} // namespace micrortps
} // namespace eprosima

#endif //_MICRORTPS_AGENT_TRANSPORT_UDP_SERVER_HPP_
