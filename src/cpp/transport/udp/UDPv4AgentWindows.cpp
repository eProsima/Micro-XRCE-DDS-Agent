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

#include <uxr/agent/transport/udp/UDPv4AgentWindows.hpp>
#include <uxr/agent/transport/util/InterfaceWindows.hpp>
#include <uxr/agent/utils/Conversion.hpp>
#include <uxr/agent/logger/Logger.hpp>

#include <string.h>

namespace eprosima {
namespace uxr {

extern template class DiscoveryServer<IPv4EndPoint>; // Explicit instantiation declaration.
extern template class DiscoveryServerWindows<IPv4EndPoint>; // Explicit instantiation declaration.

UDPv4Agent::UDPv4Agent(
        uint16_t agent_port,
        Middleware::Kind middleware_kind)
    : Server<IPv4EndPoint>{middleware_kind}
    , poll_fd_{INVALID_SOCKET, 0, 0}
    , buffer_{0}
    , agent_port_{agent_port}
#ifdef UAGENT_DISCOVERY_PROFILE
    , discovery_server_(*processor_)
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

    /* Socker initialization. */
    poll_fd_.fd = socket(PF_INET, SOCK_DGRAM, 0);

    if (INVALID_SOCKET != poll_fd_.fd)
    {
        /* IP and Port setup. */
        struct sockaddr_in address;

        ZeroMemory(&address, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_port = htons(agent_port_);
        address.sin_addr.s_addr = INADDR_ANY;

        if (SOCKET_ERROR != bind(poll_fd_.fd, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)))
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
    if (INVALID_SOCKET == poll_fd_.fd)
    {
        return true;
    }

    bool rv = false;
    if (0 == closesocket(poll_fd_.fd))
    {
        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_GREEN("server stopped"),
            "port: {}",
            agent_port_);
        poll_fd_.fd = INVALID_SOCKET;
        rv = true;
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

bool UDPv4Agent::recv_message(
        InputPacket<IPv4EndPoint>& input_packet,
        int timeout,
        TransportRc& transport_rc)
{
    bool rv = false;
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    int poll_rv = WSAPoll(&poll_fd_, 1, timeout);
    if (0 < poll_rv)
    {
        int bytes_received =
            recvfrom(
                poll_fd_.fd,
                reinterpret_cast<char*>(buffer_),
                sizeof(buffer_),
                0,
                reinterpret_cast<struct sockaddr*>(&client_addr),
                &client_addr_len);
        if (SOCKET_ERROR != bytes_received)
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
    struct sockaddr_in client_addr;

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = output_packet.destination.get_port();
    client_addr.sin_addr.s_addr = output_packet.destination.get_addr();
    int bytes_sent =
        sendto(
            poll_fd_.fd,
            reinterpret_cast<char*>(output_packet.message->get_buf()),
            int(output_packet.message->get_len()),
            0,
            reinterpret_cast<struct sockaddr*>(&client_addr),
            sizeof(client_addr));
    if (SOCKET_ERROR != bytes_sent)
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
