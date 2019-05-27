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

#include <ws2ipdef.h>
#include <MSWSock.h>
#include <functional>

#define RECEIVE_TIMEOUT 100

namespace eprosima {
namespace uxr {

DiscoveryServerWindows::DiscoveryServerWindows(const Processor& processor)
    : DiscoveryServer(processor)
    , poll_fd_{INVALID_SOCKET, 0, 0}
    , buffer_{0}
{
}

bool DiscoveryServerWindows::init(uint16_t discovery_port)
{
    bool rv = false;

    /* Socket initialization. */
    poll_fd_.fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (INVALID_SOCKET == poll_fd_.fd)
    {
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
        /* Poll setup. */
        poll_fd_.events = POLLIN;

        /* Set up multicast IP. */
        struct ip_mreq mreq;
        mreq.imr_multiaddr.s_addr = inet_addr(DISCOVERY_IP);
        mreq.imr_interface.s_addr = INADDR_ANY;
        if (SOCKET_ERROR != setsockopt(poll_fd_.fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)))
        {
            rv = true;
        }
    }

    return rv;
}

bool DiscoveryServerWindows::close()
{
    return (INVALID_SOCKET == poll_fd_.fd) || (0 == closesocket(poll_fd_.fd));
}

bool DiscoveryServerWindows::recv_message(
        InputPacket& input_packet,
        int timeout)
{
    bool rv = false;
    struct sockaddr client_addr;
    int client_addr_len = sizeof(client_addr);

    int poll_rv = WSAPoll(&poll_fd_, 1, timeout);
    if (0 < poll_rv)
    {
        int bytes_received = recvfrom(poll_fd_.fd, (char*)buffer_, sizeof(buffer_), 0, &client_addr, &client_addr_len);
        if (SOCKET_ERROR != bytes_received)
        {
            input_packet.message.reset(new InputMessage(buffer_, size_t(bytes_received)));
            uint32_t addr = ((struct sockaddr_in*)&client_addr)->sin_addr.s_addr;
            uint16_t port = ((struct sockaddr_in*)&client_addr)->sin_port;
            input_packet.source.reset(new IPv4EndPoint(addr, port));
            rv = true;
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

bool DiscoveryServerWindows::send_message(OutputPacket&& output_packet)
{
    bool rv = false;
    const IPv4EndPoint* destination = static_cast<const IPv4EndPoint*>(output_packet.destination.get());
    struct sockaddr_in client_addr;

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = destination->get_port();
    client_addr.sin_addr.s_addr = destination->get_addr();
    int bytes_sent = sendto(poll_fd_.fd,
                            (char*)output_packet.message->get_buf(),
                            output_packet.message->get_len(),
                            0,
                            (struct sockaddr*)&client_addr,
                            int(sizeof(client_addr)));
    if (SOCKET_ERROR != bytes_sent)
    {
        rv = (size_t(bytes_sent) == output_packet.message->get_len());
    }

    return rv;
}

} // namespace uxr
} // namespace eprosima
