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

#ifndef _UXR_AGENT_TRANSPORT_TCP_SERVER_HPP_
#define _UXR_AGENT_TRANSPORT_TCP_SERVER_HPP_

#include <uxr/agent/transport/tcp/TCPServer.hpp>
#include <uxr/agent/config.hpp>
#include <unordered_map>
#include <winsock2.h>
#include <vector>
#include <array>
#include <list>
#include <set>

namespace eprosima {
namespace uxr {

struct TCPConnection
{
    struct pollfd* poll_fd;
    TCPInputBuffer input_buffer;
    uint32_t addr;
    uint16_t port;
    uint32_t id;
    bool active;
    std::mutex mtx;
};

class TCPServerBase : public TCPServerBase
{
public:
    microxrcedds_agent_DllAPI TCPServerBase(uint16_t port);
    microxrcedds_agent_DllAPI ~TCPServerBase() = default;

private:
    virtual bool init() override;
    virtual bool close() override;
    virtual bool recv_message(InputPacket& input_packet, int timeout) override;
    virtual bool send_message(OutputPacket output_packet) override;
    virtual int get_error() override;
    bool read_message(int timeout);
    uint16_t read_data(TCPConnection& connection);
    bool open_connection(SOCKET fd, struct sockaddr_in* sockaddr);
    bool close_connection(TCPConnection& connection);
    bool connection_available();
    void listener_loop();
    static void init_input_buffer(TCPInputBuffer& buffer);
    static int recv_locking(TCPConnection& connection, char* buffer, int len);
    static int send_locking(TCPConnection& connection, char* buffer, int len);

private:
    std::array<TCPConnection, TCP_MAX_CONNECTIONS> connections_;
    std::set<uint32_t> active_connections_;
    std::list<uint32_t> free_connections_;
    std::mutex connections_mtx_;
    struct pollfd listener_poll_;
    std::array<struct pollfd, TCP_MAX_CONNECTIONS> poll_fds_;
    uint8_t buffer_[TCP_TRANSPORT_MTU];
    std::unique_ptr<std::thread> listener_thread_;
    std::atomic<bool> running_cond_;
    std::queue<InputPacket> messages_queue_;
};

} // namespace uxr
} // namespace eprosima

#endif //_UXR_AGENT_TRANSPORT_TCP_SERVER_HPP_
