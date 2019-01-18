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

#include <uxr/agent/transport/tcp/TCPServerBase.hpp>
#include <uxr/agent/transport/discovery/DiscoveryServerLinux.hpp>
#include <uxr/agent/config.hpp>
#include <netinet/in.h>
#include <sys/poll.h>
#include <array>
#include <list>
#include <set>

namespace eprosima {
namespace uxr {

class TCPConnectionPlatform : public TCPConnection
{
public:
    TCPConnectionPlatform() = default;
    ~TCPConnectionPlatform() = default;

public:
    struct pollfd* poll_fd;
};

class TCPServer : public TCPServerBase
{
public:
    TCPServer(uint16_t port, uint16_t discovery_port = UXR_DEFAULT_DISCOVERY_PORT);
    ~TCPServer() = default;

private:
    bool init(bool discovery_enabled) final;
    bool close() final;
    bool recv_message(InputPacket& input_packet, int timeout) final;
    bool send_message(OutputPacket output_packet) final;
    int get_error() final;
    bool read_message(int timeout);
    bool open_connection(int fd, struct sockaddr_in* sockaddr);
    bool connection_available();
    void listener_loop();
    static void init_input_buffer(TCPInputBuffer& buffer);
    static void sigpipe_handler(int fd) { (void)fd; }

    bool close_connection(TCPConnection& connection) override;
    size_t recv_locking(TCPConnection& connection, uint8_t* buffer, size_t len, uint8_t& errcode) override;
    size_t send_locking(TCPConnection& connection, uint8_t* buffer, size_t len, uint8_t& errcode) override;

private:
    std::array<TCPConnectionPlatform, TCP_MAX_CONNECTIONS> connections_;
    std::set<uint32_t> active_connections_;
    std::list<uint32_t> free_connections_;
    std::mutex connections_mtx_;
    struct pollfd listener_poll_;
    std::array<struct pollfd, TCP_MAX_CONNECTIONS> poll_fds_;
    uint8_t buffer_[UINT16_MAX];
    std::unique_ptr<std::thread> listener_thread_;
    std::atomic<bool> running_cond_;
    std::queue<InputPacket> messages_queue_;
    DiscoveryServerLinux discovery_server_;
};

} // namespace uxr
} // namespace eprosima

#endif //_UXR_AGENT_TRANSPORT_TCP_SERVER_HPP_
