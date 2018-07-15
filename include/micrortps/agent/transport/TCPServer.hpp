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

#ifndef _MICRORTPS_AGENT_TRANSPORT_TCP_SERVER_HPP_
#define _MICRORTPS_AGENT_TRANSPORT_TCP_SERVER_HPP_

#include <micrortps/agent/transport/Server.hpp>
#include <sys/poll.h>
#include <vector>
#include <array>

#define TCP_TRANSPORT_MTU 512
#define MICRORTPS_MAX_TCP_CLIENTS 1
#define MICRORTPS_MAX_BACKLOG_TCP_CONNECTIONS 100

namespace eprosima {
namespace micrortps {

/******************************************************************************
 * TCP Client.
 ******************************************************************************/
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

class TCPEndPoint : public EndPoint
{
public:
    TCPEndPoint() {}
    ~TCPEndPoint() {}

public:
    struct pollfd* poll_fd;
    TCPEndPoint* next;
    TCPEndPoint* prev;
    TCPInputBuffer input_buffer;
};

/******************************************************************************
 * TCP Server.
 ******************************************************************************/
class TCPServer : public Server
{
public:
    TCPServer() : clients_{}, poll_fds_{}, buffer_{0} {}
    ~TCPServer() {}

    virtual bool recv_message(InputPacket& input_packet, int timeout) override;
    virtual bool send_message(OutputPacket output_packet) override;
    virtual int get_error() override;
    int launch(uint16_t port);

private:
    uint16_t read_data(TCPEndPoint* client);
    void disconnect_client(TCPEndPoint* client);
    static void init_input_buffer(TCPInputBuffer* buffer);
    static void sigpipe_handler(int fd) { (void)fd; } // TODO (julian): handle sigpipe to disconnect.

private:
    std::array<TCPEndPoint, MICRORTPS_MAX_TCP_CLIENTS> clients_;
    TCPEndPoint* connected_clients_;
    TCPEndPoint* available_clients_;
    TCPEndPoint* last_client_read_;
    std::array<struct pollfd, MICRORTPS_MAX_TCP_CLIENTS + 1> poll_fds_;
    uint8_t buffer_[1024];
};

} // namespace micrortps
} // namespace eprosima

#endif //_MICRORTPS_AGENT_TRANSPORT_TCP_SERVER_HPP_
