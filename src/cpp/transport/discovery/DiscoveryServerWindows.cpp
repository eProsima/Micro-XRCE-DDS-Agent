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

#include <uxr/agent/transport/discovery/DiscoveryServerWindows.hpp>
#include <uxr/agent/transport/endpoint/IPv4EndPoint.hpp>
#include <uxr/agent/processor/Processor.hpp>
#include <uxr/agent/logger/Logger.hpp>

#include <ws2ipdef.h>
#include <MSWSock.h>
#include <iphlpapi.h>
#include <functional>

#define RECEIVE_TIMEOUT 100

namespace eprosima {
namespace uxr {

template<typename EndPoint>
DiscoveryServerWindows<EndPoint>::DiscoveryServerWindows(
        const Processor<EndPoint>& processor)
    : DiscoveryServer<EndPoint>(processor)
    , poll_fd_{INVALID_SOCKET, 0, 0}
    , buffer_{0}
{}

template<typename EndPoint>
bool DiscoveryServerWindows<EndPoint>::init(
        uint16_t discovery_port)
{
    bool rv = false;

    /* Socket initialization. */
    poll_fd_.fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (INVALID_SOCKET == poll_fd_.fd)
    {
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("socket error"),
            "Port: {}",
            discovery_port);
        return false;
    }

    /* Local IP and Port setup. */
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(discovery_port);
    address.sin_addr.s_addr = INADDR_ANY;
    memset(address.sin_zero, '\0', sizeof(address.sin_zero));
    if (SOCKET_ERROR != bind(poll_fd_.fd, (struct sockaddr*)&address, sizeof(address)))
    {
        /* Log. */
        UXR_AGENT_LOG_DEBUG(
            UXR_DECORATE_GREEN("port opened"),
            "Port: {}",
            discovery_port);

        /* Poll setup. */
        poll_fd_.events = POLLIN;

        /* Set up multicast IP. */
        struct ip_mreq mreq;
        mreq.imr_multiaddr.s_addr = inet_addr(DISCOVERY_IP);
        mreq.imr_interface.s_addr = INADDR_ANY;
        if (SOCKET_ERROR != setsockopt(poll_fd_.fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)))
        {
            DiscoveryServer<EndPoint>::discovery_port_ = discovery_port;
            rv = true;
            UXR_AGENT_LOG_INFO(
                UXR_DECORATE_GREEN("running..."),
                "Port: {}",
                discovery_port);
        }
        else
        {
            UXR_AGENT_LOG_ERROR(
                UXR_DECORATE_RED("socket opt error"),
                "Port: {}",
                discovery_port);
        }
    }

    return rv;
}

template<typename EndPoint>
bool DiscoveryServerWindows<EndPoint>::close()
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
            DiscoveryServer<EndPoint>::discovery_port_);
        poll_fd_.fd = INVALID_SOCKET;
        rv = true;
    }
    else
    {
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("socket error"),
            "port: {}",
            DiscoveryServer<EndPoint>::discovery_port_);
    }
    return rv;
}

template<typename EndPoint>
bool DiscoveryServerWindows<EndPoint>::recv_message(
        InputPacket<IPv4EndPoint>& input_packet,
        int timeout)
{
    bool rv = false;
    struct sockaddr client_addr;
    int client_addr_len = sizeof(client_addr);

    int poll_rv = WSAPoll(&poll_fd_, 1, timeout);
    if (0 < poll_rv)
    {
        int bytes_received =
                recvfrom(poll_fd_.fd,
                         reinterpret_cast<char*>(buffer_),
                         sizeof(buffer_),
                         0,
                         &client_addr, &client_addr_len);

        if (SOCKET_ERROR != bytes_received)
        {
            std::array<uint8_t, 4> remote_addr{
                uint8_t(client_addr.sa_data[2]),
                uint8_t(client_addr.sa_data[3]),
                uint8_t(client_addr.sa_data[4]),
                uint8_t(client_addr.sa_data[5])};
            uint16_t remote_port = reinterpret_cast<sockaddr_in*>(&client_addr)->sin_port;

            bool addr_filtered = false;
            for (const auto& a : this->transport_addresses_)
            {
                if (dds::xrce::ADDRESS_FORMAT_MEDIUM == a._d())
                {
                    if (a.medium_locator().address() == remote_addr)
                    {
                        addr_filtered = true;
                        break;
                    }
                }
            }

            if (!(addr_filtered && remote_port == htons(this->filter_port_)))
            {
                input_packet.message.reset(new InputMessage(buffer_, size_t(bytes_received)));
                uint32_t addr = (reinterpret_cast<struct sockaddr_in*>(&client_addr))->sin_addr.s_addr;
                uint16_t port = (reinterpret_cast<struct sockaddr_in*>(&client_addr))->sin_port;
                input_packet.source = IPv4EndPoint(addr, port);
                rv = true;

                UXR_AGENT_LOG_MESSAGE(
                    UXR_DECORATE_YELLOW("[==>> UDP <<==]"),
                    input_packet.source.get_addr(),
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

template<typename EndPoint>
bool DiscoveryServerWindows<EndPoint>::send_message(
        OutputPacket<IPv4EndPoint>&& output_packet)
{
    bool rv = false;
    struct sockaddr_in client_addr;

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = output_packet.destination.get_port();
    client_addr.sin_addr.s_addr = output_packet.destination.get_addr();
    int bytes_sent =
            sendto(poll_fd_.fd,
                   reinterpret_cast<char*>(output_packet.message->get_buf()),
                   int(output_packet.message->get_len()),
                   0,
                   reinterpret_cast<struct sockaddr*>(&client_addr),
                   int(sizeof(client_addr)));
    if (SOCKET_ERROR != bytes_sent)
    {
        rv = (size_t(bytes_sent) == output_packet.message->get_len());

        UXR_AGENT_LOG_MESSAGE(
            UXR_DECORATE_YELLOW("[** <<UDP>> **]"),
            output_packet.destination.get_addr(),
            output_packet.message->get_buf(),
            output_packet.message->get_len());
    }

    return rv;
}

template class DiscoveryServerWindows<IPv4EndPoint>;
template class DiscoveryServerWindows<IPv6EndPoint>;

} // namespace uxr
} // namespace eprosima
