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

#ifndef UXR_AGENT_TRANSPORT_TCPv4_AGENT_HPP_
#define UXR_AGENT_TRANSPORT_TCPv4_AGENT_HPP_

#include <uxr/agent/transport/tcp/TCPServerBase.hpp>
#include <uxr/agent/transport/Server.hpp>
#ifdef UAGENT_DISCOVERY_PROFILE
#include <uxr/agent/transport/discovery/DiscoveryServerLinux.hpp>
#endif

#include <netinet/in.h>
#include <sys/poll.h>
#include <array>
#include <list>
#include <set>
#include <queue>

namespace eprosima {
namespace uxr {

struct TCPv4ConnectionLinux : public TCPv4Connection
{
    struct pollfd* poll_fd;
};

extern template class Server<IPv4EndPoint>; // Explicit instantiation declaration.

class TCPv4Agent : public Server<IPv4EndPoint>, public TCPServerBase<TCPv4ConnectionLinux>
{
public:
    TCPv4Agent(
            uint16_t agent_port,
            Middleware::Kind middleware_kind);

    ~TCPv4Agent() final;

#ifdef UAGENT_DISCOVERY_PROFILE
    bool has_discovery() final { return true; }
#endif

#ifdef UAGENT_P2P_PROFILE
    bool has_p2p() final { return true; }
#endif

private:
    bool init() final;

    bool fini() final;

#ifdef UAGENT_DISCOVERY_PROFILE
    bool init_discovery(uint16_t discovery_port) final;

    bool fini_discovery() final;
#endif

#ifdef UAGENT_P2P_PROFILE
    bool init_p2p(uint16_t p2p_port) final;

    bool fini_p2p() final;
#endif

    bool recv_message(
            InputPacket<IPv4EndPoint>& input_packet,
            int timeout,
            TransportRc& transport_rc) final;

    bool send_message(
            OutputPacket<IPv4EndPoint> output_packet,
            TransportRc& transport_rc) final;

    bool handle_error(
            TransportRc transport_rc) final;

    bool read_message(
            int timeout,
            TransportRc& transport_rc);

    bool open_connection(
            int fd,
            struct sockaddr_in& sockaddr);

    bool close_connection(
            TCPv4ConnectionLinux& connection);

    bool connection_available();

    void listener_loop();

    static void init_input_buffer(
            TCPInputBuffer& buffer);

    static void sigpipe_handler(int fd) { (void)fd; }

    size_t recv_data(
            TCPv4ConnectionLinux& connection,
            uint8_t* buffer,
            size_t len,
            TransportRc& transport_rc) final;

    size_t send_data(
            TCPv4ConnectionLinux& connection,
            uint8_t* buffer,
            size_t len,
            TransportRc& transport_rc) final;

private:
    std::array<TCPv4ConnectionLinux, TCP_MAX_CONNECTIONS> connections_;
    std::set<uint32_t> active_connections_;
    std::list<uint32_t> free_connections_;
    std::map<IPv4EndPoint, uint32_t> endpoint_to_connection_map_;
    std::mutex connections_mtx_;
    struct pollfd listener_poll_;
    std::array<struct pollfd, TCP_MAX_CONNECTIONS> poll_fds_;
    uint8_t buffer_[SERVER_BUFFER_SIZE];
    uint16_t agent_port_;
    std::thread listener_thread_;
    std::atomic<bool> running_cond_;
    std::queue<InputPacket<IPv4EndPoint>> messages_queue_;
#ifdef UAGENT_DISCOVERY_PROFILE
    DiscoveryServerLinux<IPv4EndPoint> discovery_server_;
#endif
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_TRANSPORT_TCPv4_AGENT_HPP_
