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

#ifndef _MICRORTPS_AGENT_TRANSPORT_TCPSERVER_HPP_
#define _MICRORTPS_AGENT_TRANSPORT_TCPSERVER_HPP_

#include <stdint.h>
#include <stddef.h>
#include <sys/poll.h>

#define MICRORTPS_MAX_TCP_CLIENTS 1024

namespace eprosima {
namespace micrortps {

class TCPServer
{
public:
    static TCPServer* create(uint16_t port);

    bool send_data(uint32_t addr, uint16_t port, const uint8_t* buf, size_t len);
    bool recv_data(uint32_t* addr, uint16_t* port, uint8_t** buf, size_t* len, int timeout);
    int get_error();

private:
    TCPServer() : listen_fd_(0), poll_fd_{}, buffer_{0} {}
    ~TCPServer();
    int init(uint16_t port);

private:
    int listen_fd_;
    int client_fds_[MICRORTPS_MAX_TCP_CLIENTS];
    struct pollfd poll_fd_[MICRORTPS_MAX_TCP_CLIENTS];
    uint8_t buffer_[1024];
};

} // namespace micrortps
} // namespace eprosima

#endif //_MICRORTPS_AGENT_TRANSPORT_TCPSERVER_HPP_
