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

#include <uxr/agent/transport/udp/UDPv4AgentLinux.hpp>
#include <uxr/agent/transport/util/InterfaceLinux.hpp>
#include <uxr/agent/utils/Conversion.hpp>
#include <uxr/agent/logger/Logger.hpp>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <cerrno>

namespace eprosima {
namespace uxr {

#ifdef UAGENT_DISCOVERY_PROFILE
extern template class DiscoveryServer<IPv4EndPoint>; // Explicit instantiation declaration.
extern template class DiscoveryServerLinux<IPv4EndPoint>; // Explicit instantiation declaration.
#endif // UAGENT_DISCOVERY_PROFILE

UDPv4Agent::UDPv4Agent(
        uint16_t agent_port,
        Middleware::Kind middleware_kind)
    : Server<IPv4EndPoint>{middleware_kind}
    , poll_fd_{-1, 0, 0}
    , buffer_{0}
    , agent_port_{agent_port}
#ifdef UAGENT_DISCOVERY_PROFILE
    , discovery_server_{*processor_}
#endif
#ifdef UAGENT_P2P_PROFILE
    , agent_discoverer_{*this}
#endif
{}

UDPv4Agent::~UDPv4Agent()
{
    try
    {
        stop();
    }
    catch (std::exception& e)
    {
        UXR_AGENT_LOG_CRITICAL(
            UXR_DECORATE_RED("error stopping server"),
            "exception: {}",
            e.what());
    }
}

bool UDPv4Agent::init()
{
    bool rv = false;

    poll_fd_.fd = socket(PF_INET, SOCK_DGRAM, 0);

    if (-1 != poll_fd_.fd)
    {
        struct sockaddr_in address{};

        address.sin_family = AF_INET;
        address.sin_port = htons(agent_port_);
        address.sin_addr.s_addr = INADDR_ANY;
        memset(address.sin_zero, '\0', sizeof(address.sin_zero));

        if (-1 != bind(poll_fd_.fd, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)))
        {
            poll_fd_.events = POLLIN;
            rv = true;

            UXR_AGENT_LOG_DEBUG(
                UXR_DECORATE_GREEN("port opened"),
                "port: {}",
                agent_port_);

            UXR_AGENT_LOG_INFO(
                UXR_DECORATE_GREEN("running..."),
                "port: {}",
                agent_port_);
        }
        else
        {
            UXR_AGENT_LOG_ERROR(
                UXR_DECORATE_RED("bind error"),
                "port: {}, errno: {}",
                agent_port_, errno);
        }
    }
    else
    {
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("socket error"),
            "port: {}, errno: {}",
            agent_port_, errno);
    }

    return rv;
}

bool UDPv4Agent::fini()
{
    if (-1 == poll_fd_.fd)
    {
        return true;
    }

    bool rv = false;
    if (0 == ::close(poll_fd_.fd))
    {
        poll_fd_.fd = -1;
        rv = true;
        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_GREEN("server stopped"),
            "port: {}",
            agent_port_);
    }
    else
    {
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("socket error"),
            "port: {}, errno: {}",
            agent_port_, errno);
    }
    return rv;
}

#ifdef UAGENT_DISCOVERY_PROFILE
bool UDPv4Agent::init_discovery(uint16_t discovery_port)
{
    std::vector<dds::xrce::TransportAddress> transport_addresses;
    util::get_transport_interfaces<IPv4EndPoint>(this->agent_port_, transport_addresses);
    return discovery_server_.run(discovery_port, transport_addresses);
}

bool UDPv4Agent::fini_discovery()
{
    return discovery_server_.stop();
}
#endif

#ifdef UAGENT_P2P_PROFILE
bool UDPv4Agent::init_p2p(uint16_t p2p_port)
{
#ifdef UAGENT_DISCOVERY_PROFILE
    discovery_server_.set_filter_port(p2p_port);
#endif
    return agent_discoverer_.start(p2p_port, agent_port_);
}

bool UDPv4Agent::fini_p2p()
{
#ifdef UAGENT_DISCOVERY_PROFILE
    discovery_server_.set_filter_port(0);
#endif
    return agent_discoverer_.stop();
}
#endif

bool UDPv4Agent::recv_message(
        InputPacket<IPv4EndPoint>& input_packet,
        int timeout,
        TransportRc& transport_rc)
{
    bool rv = false;
    struct sockaddr_in client_addr{};
    socklen_t client_addr_len = sizeof(struct sockaddr_in);

    int poll_rv = poll(&poll_fd_, 1, timeout);
    if (0 < poll_rv)
    {
        ssize_t bytes_received =
                recvfrom(poll_fd_.fd,
                         buffer_,
                         sizeof(buffer_),
                         0,
                         reinterpret_cast<struct sockaddr*>(&client_addr),
                         &client_addr_len);
        if (-1 != bytes_received)
        {
            input_packet.message.reset(new InputMessage(buffer_, size_t(bytes_received)));
            uint32_t addr = client_addr.sin_addr.s_addr;
            uint16_t port = client_addr.sin_port;
            input_packet.source = IPv4EndPoint(addr, port);
            rv = true;

            uint32_t raw_client_key = 0u;
            Server<IPv4EndPoint>::get_client_key(input_packet.source, raw_client_key);
            UXR_AGENT_LOG_MESSAGE(
                UXR_DECORATE_YELLOW("[==>> UDP <<==]"),
                raw_client_key,
                input_packet.message->get_buf(),
                input_packet.message->get_len());
        }
        else
        {
            transport_rc = TransportRc::server_error;
        }
    }
    else
    {
        transport_rc = (0 == poll_rv) ? TransportRc::timeout_error : TransportRc::server_error;
    }

    return rv;
}

bool UDPv4Agent::send_message(
        OutputPacket<IPv4EndPoint> output_packet,
        TransportRc& transport_rc)
{
    bool rv = false;
    struct sockaddr_in client_addr{};

    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = output_packet.destination.get_port();
    client_addr.sin_addr.s_addr = output_packet.destination.get_addr();

    ssize_t bytes_sent =
        sendto(
            poll_fd_.fd,
            output_packet.message->get_buf(),
            output_packet.message->get_len(),
            0,
            reinterpret_cast<struct sockaddr*>(&client_addr),
            sizeof(client_addr));
    if (-1 != bytes_sent)
    {
        if (size_t(bytes_sent) == output_packet.message->get_len())
        {
            rv = true;
            uint32_t raw_client_key = 0u;
            Server<IPv4EndPoint>::get_client_key(output_packet.destination, raw_client_key);
            UXR_AGENT_LOG_MESSAGE(
                UXR_DECORATE_YELLOW("[** <<UDP>> **]"),
                raw_client_key,
                output_packet.message->get_buf(),
                output_packet.message->get_len());
        }
    }
    else
    {
        transport_rc = TransportRc::server_error;
    }

    return rv;
}

bool UDPv4Agent::handle_error(
        TransportRc /*transport_rc*/)
{
    return fini() && init();
}

} // namespace uxr
} // namespace eprosima
