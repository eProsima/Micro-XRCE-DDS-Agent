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
#include <vector>
#include <array>

#define TCP_TRANSPORT_MTU 512
#define MICRORTPS_MAX_TCP_CLIENTS 1
#define MICRORTPS_MAX_BACKLOG_TCP_CONNECTIONS 100

namespace eprosima {
namespace micrortps {

typedef enum TCPInputBufferState
{
    TCP_BUFFER_EMPTY,
    TCP_SIZE_INCOMPLETE,
    TCP_SIZE_READ,
    TCP_MESSAGE_INCOMPLETE,
    TCP_MESSAGE_AVAILABLE
} TCPInputBufferState;

typedef struct TCPInputBuffer TCPInputBuffer;
struct TCPInputBuffer
{
    std::vector<uint8_t> buffer;
    uint16_t position;
    TCPInputBufferState state;
    uint16_t msg_size;
};

typedef struct TCPClient TCPClient;
struct TCPClient
{
    struct pollfd* poll_fd;
    TCPClient* next;
    TCPClient* prev;
    TCPInputBuffer input_buffer;
};

class TCPServer
{
public:
    static TCPServer* create(uint16_t port);

    bool send_msg(const uint8_t* buf, size_t len, TCPClient* client);
    bool recv_msg(uint8_t** buf, size_t* len, int timeout, TCPClient** client);
    int get_error();

private:
    TCPServer() : clients_{}, poll_fds_{}, buffer_{0} {}
    ~TCPServer();
    int init(uint16_t port);
    uint16_t read_data(TCPClient* client);
    void disconnect_client(TCPClient* client);
    static void init_input_buffer(TCPInputBuffer* buffer);

private:
    std::array<TCPClient, MICRORTPS_MAX_TCP_CLIENTS> clients_;
    TCPClient* connected_clients_;
    TCPClient* available_clients_;
    TCPClient* last_client_read_;
    std::array<struct pollfd, MICRORTPS_MAX_TCP_CLIENTS + 1> poll_fds_;
    uint8_t buffer_[1024];
};

} // namespace micrortps
} // namespace eprosima

#endif //_MICRORTPS_AGENT_TRANSPORT_TCPSERVER_HPP_
