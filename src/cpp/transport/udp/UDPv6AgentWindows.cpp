// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <uxr/agent/transport/udp/UDPv6AgentWindows.hpp>
#include <uxr/agent/utils/Conversion.hpp>
#include <uxr/agent/logger/Logger.hpp>

#include <string.h>
#include <ws2ipdef.h>

namespace eprosima {
namespace uxr {

extern template class DiscoveryServer<IPv6EndPoint>; // Explicit instantiation declaration.
extern template class DiscoveryServerWindows<IPv6EndPoint>; // Explicit instantiation declaration.

UDPv6Agent::UDPv6Agent(
        uint16_t agent_port,
        Middleware::Kind middleware_kind)
    : Server<IPv6EndPoint>{middleware_kind}
    , poll_fd_{INVALID_SOCKET, 0, 0}
    , buffer_{0}
#ifdef UAGENT_DISCOVERY_PROFILE
    , discovery_server_(*processor_)
#endif
{
    dds::xrce::TransportAddressLarge large_locator;
    large_locator.port(agent_port);
    transport_address_.large_locator(large_locator);
}

UDPv6Agent::~UDPv6Agent()
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

bool UDPv6Agent::init()
{
    bool rv = false;

    /* Socker initialization. */
    poll_fd_.fd = socket(PF_INET6, SOCK_DGRAM, 0);

    if (INVALID_SOCKET != poll_fd_.fd)
    {
        /* IP and Port setup. */
        struct sockaddr_in6 address;
        address.sin6_family = AF_INET6;
        address.sin6_port = htons(uint16_t(transport_address_.large_locator().port()));
        address.sin6_addr = in6addr_any;

        if (SOCKET_ERROR != bind(poll_fd_.fd, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)))
        {
            poll_fd_.events = POLLIN;
            rv = true;

            UXR_AGENT_LOG_DEBUG(
                UXR_DECORATE_GREEN("port opened"),
                "port: {}",
                transport_address_.large_locator().port()
                );

            UXR_AGENT_LOG_INFO(
                UXR_DECORATE_GREEN("running..."),
                "port: {}",
                transport_address_.large_locator().port());
        }
        else
        {
            UXR_AGENT_LOG_ERROR(
                UXR_DECORATE_RED("bind error"),
                "port: {}",
                transport_address_.large_locator().port());
        }
    }
    else
    {
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("socket error"),
            "port: {}",
            transport_address_.large_locator().port());
    }

    return rv;
}

bool UDPv6Agent::close()
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
            transport_address_.large_locator().port());
        poll_fd_.fd = INVALID_SOCKET;
        rv = true;
    }
    else
    {
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("socket error"),
            "port: {}",
            transport_address_.large_locator().port());
    }
    return rv;
}

#ifdef UAGENT_DISCOVERY_PROFILE
bool UDPv6Agent::init_discovery(uint16_t discovery_port)
{
    return discovery_server_.run(discovery_port);
}

bool UDPv6Agent::close_discovery()
{
    return discovery_server_.stop();
}
#endif

bool UDPv6Agent::recv_message(
        InputPacket<IPv6EndPoint>& input_packet,
        int timeout)
{
    bool rv = false;
    struct sockaddr_in6 client_addr;
    int client_addr_len = sizeof(client_addr);

    int poll_rv = WSAPoll(&poll_fd_, 1, timeout);
    if (0 < poll_rv)
    {
        int bytes_received =
                recvfrom(poll_fd_.fd,
                         reinterpret_cast<char*>(buffer_),
                         sizeof(buffer_),
                         0,
                         reinterpret_cast<struct sockaddr*>(&client_addr),
                         &client_addr_len);
        if (SOCKET_ERROR != bytes_received)
        {
            input_packet.message.reset(new InputMessage(buffer_, size_t(bytes_received)));
            std::array<uint8_t, 16> addr{};
            std::copy(std::begin(client_addr.sin6_addr.s6_addr), std::end(client_addr.sin6_addr.s6_addr), addr.begin());
            input_packet.source = IPv6EndPoint(addr, client_addr.sin6_port);
            rv = true;

            uint32_t raw_client_key;
            if (Server<IPv6EndPoint>::get_client_key(input_packet.source, raw_client_key))
            {
                UXR_AGENT_LOG_MESSAGE(
                    UXR_DECORATE_YELLOW("[==>> UDP <<==]"),
                    raw_client_key,
                    input_packet.message->get_buf(),
                    input_packet.message->get_len());
            }
        }
    }
    else
    {
        if (0 == poll_rv)
        {
            WSASetLastError(WAIT_TIMEOUT);
        }
    }

    return rv;
}

bool UDPv6Agent::send_message(
        OutputPacket<IPv6EndPoint> output_packet)
{
    bool rv = false;
    struct sockaddr_in6 client_addr{};

    client_addr.sin6_family = AF_INET6;
    client_addr.sin6_port = output_packet.destination.get_port();
    const std::array<uint8_t, 16>& destination = output_packet.destination.get_addr();
    std::copy(destination.begin(), destination.end(), std::begin(client_addr.sin6_addr.s6_addr));

    int bytes_sent =
            sendto(poll_fd_.fd,
                   reinterpret_cast<char*>(output_packet.message->get_buf()),
                   int(output_packet.message->get_len()),
                   0,
                   reinterpret_cast<struct sockaddr*>(&client_addr),
                   sizeof(client_addr));
    if (SOCKET_ERROR != bytes_sent)
    {
        if (size_t(bytes_sent) != output_packet.message->get_len())
        {
            rv = true;

            uint32_t raw_client_key;
            if (Server<IPv6EndPoint>::get_client_key(output_packet.destination, raw_client_key))
            {
                UXR_AGENT_LOG_MESSAGE(
                    UXR_DECORATE_YELLOW("[** <<UDP>> **]"),
                    raw_client_key,
                    output_packet.message->get_buf(),
                    output_packet.message->get_len());
            }
        }
    }

    return rv;
}

int UDPv6Agent::get_error()
{
    return WSAGetLastError();
}

} // namespace uxr
} // namespace eprosima
