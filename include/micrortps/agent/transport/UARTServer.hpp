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

#ifndef _MICRORTPS_AGENT_TRANSPORT_UART_SERVER_HPP_
#define _MICRORTPS_AGENT_TRANSPORT_UART_SERVER_HPP_

#include <micrortps/agent/transport/Server.hpp>
#include <micrortps/agent/transport/SerialLayer.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/poll.h>
#include <map>

namespace eprosima {
namespace micrortps {

/******************************************************************************
 * UART Client.
 ******************************************************************************/
class UARTEndPoint : public EndPoint
{
public:
    UARTEndPoint(uint8_t addr) { addr_ = addr; }
    ~UARTEndPoint() {}

public:
    uint8_t addr_;
};

/******************************************************************************
 * UART Server.
 ******************************************************************************/
class UARTServer : public Server
{
public:
    UARTServer() : poll_fd_{}, buffer_{0}, serial_io_{}, errno_(0), clients_{} {}
    ~UARTServer() {}

    virtual bool recv_message(InputPacket& input_packet, int timeout) override;
    virtual bool send_message(OutputPacket output_packet) override;
    virtual int get_error() override;
    int launch(int fd, uint8_t addr);

private:
    int init(int fd, uint8_t addr);
    static uint16_t read_data(void* instance, uint8_t* buf, size_t len, int timeout);

private:
    uint8_t addr_;
    struct pollfd poll_fd_;
    uint8_t buffer_[MICRORTPS_SERIAL_MTU];
    SerialIO serial_io_;
    int errno_;
    std::map<uint8_t, UARTEndPoint*> clients_;
};

} // namespace micrortps
} // namespace eprosima

#endif //_MICRORTPS_AGENT_TRANSPORT_UART_SERVER_HPP_
