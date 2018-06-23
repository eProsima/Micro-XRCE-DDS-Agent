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

#ifndef _MICRORTPS_AGENT_TRANSPORT_UARTSERVER_HPP_
#define _MICRORTPS_AGENT_TRANSPORT_UARTSERVER_HPP_

#include <agent/transport/SerialLayer.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/poll.h>

namespace eprosima {
namespace micrortps {

class UARTServer
{
public:
    static UARTServer* create(int fd, uint8_t addr);

    bool send_msg(const uint8_t* buf, size_t len, uint8_t addr);
    bool recv_msg(uint8_t** buf, size_t* len, uint8_t* addr, int timeout);
    int get_error();

private:
    UARTServer() : fd_(0), poll_fd_{}, buffer_{0}, serial_io_{}, errno_(0) {}
    ~UARTServer();
    int init(int fd, uint8_t addr);
    static uint16_t read_data(void* instance, uint8_t* buf, size_t len, int timeout);

private:
    int fd_;
    uint8_t addr_;
    struct pollfd poll_fd_;
    uint8_t buffer_[MICRORTPS_SERIAL_MTU];
    SerialIO serial_io_;
    int errno_;
};

} // namespace micrortps
} // namespace eprosima

#endif //_MICRORTPS_AGENT_TRANSPORT_UARTSERVER_HPP_
