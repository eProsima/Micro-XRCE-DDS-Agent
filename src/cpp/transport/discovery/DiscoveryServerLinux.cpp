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

#include <uxr/agent/transport/discovery/DiscoveryServerLinux.hpp>
#include <uxr/agent/transport/udp/UDPEndPoint.hpp>
#include <uxr/agent/processor/Processor.hpp>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define RECEIVE_TIMEOUT 100

namespace eprosima {
namespace uxr {

DiscoveryServerLinux::DiscoveryServerLinux(
        const Processor& processor,
        uint16_t agent_port)
    : DiscoveryServer(processor, agent_port)
    , poll_fd_{-1, 0, 0}
    , buffer_{0}
{
}

bool DiscoveryServerLinux::init(uint16_t discovery_port)
{
    bool rv = false;

    /* Socket initialization. */
    poll_fd_.fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (-1 == poll_fd_.fd)
    {
        return false;
    }

    /* Set reuse port. */
    int reuse = 1;
    if (-1 == setsockopt(poll_fd_.fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)))
    {
        return false;
    }

    /* Set reuse port. */
    int reuse = 1;
    if (-1 == setsockopt(poll_fd_.fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)))
    {
        return false;
    }

    /* Local IP and Port setup. */
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(discovery_port);
    address.sin_addr.s_addr = INADDR_ANY;
    memset(address.sin_zero, '\0', sizeof(address.sin_zero));
    if (-1 != bind(poll_fd_.fd, (struct sockaddr*)&address, sizeof(address)))
    {
        /* Poll setup. */
        poll_fd_.events = POLLIN;

        /* Set up multicast IP. */
        struct ip_mreq mreq;
        mreq.imr_multiaddr.s_addr = inet_addr(DISCOVERY_IP);
        mreq.imr_interface.s_addr = INADDR_ANY;
        if (-1 != setsockopt(poll_fd_.fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)))
        {
            /* Get local address. */
            int fd = socket(PF_INET, SOCK_DGRAM, 0);
            struct sockaddr_in temp_addr;
            temp_addr.sin_family = AF_INET;
            temp_addr.sin_port = htons(80);
            temp_addr.sin_addr.s_addr = inet_addr("1.2.3.4");
            int connected = connect(fd, (struct sockaddr *)&temp_addr, sizeof(temp_addr));
            if (0 == connected)
            {
                struct sockaddr local_addr;
                socklen_t local_addr_len = sizeof(local_addr);
                if (-1 != getsockname(fd, &local_addr, &local_addr_len))
                {
                    transport_address_.medium_locator().address({uint8_t(local_addr.sa_data[2]),
                                                                 uint8_t(local_addr.sa_data[3]),
                                                                 uint8_t(local_addr.sa_data[4]),
                                                                 uint8_t(local_addr.sa_data[5])});
                    rv = true;
                }
                ::close(fd);
            }
        }
    }

    return rv;
}

bool DiscoveryServerLinux::close()
{
    return (-1 == poll_fd_.fd) || (0 == ::close(poll_fd_.fd));
}

bool DiscoveryServerLinux::recv_message(
        InputPacket& input_packet,
        int timeout)
{
    bool rv = false;
    struct sockaddr client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int poll_rv = poll(&poll_fd_, 1, timeout);
    if (0 < poll_rv)
    {
        ssize_t bytes_received = recvfrom(poll_fd_.fd, buffer_, sizeof(buffer_), 0, &client_addr, &client_addr_len);
        if (0 < bytes_received)
        {
            std::array<uint8_t, 4> remote_addr{
                uint8_t(client_addr.sa_data[2]),
                uint8_t(client_addr.sa_data[3]),
                uint8_t(client_addr.sa_data[4]),
                uint8_t(client_addr.sa_data[5])
            };
            uint16_t remote_port = ((struct sockaddr_in*)&client_addr)->sin_port;

            if (remote_addr != transport_address_.medium_locator().address() ||
                remote_port != htons(filter_port_))
            {
                input_packet.message.reset(new InputMessage(buffer_, size_t(bytes_received)));
                uint32_t addr = ((struct sockaddr_in*)&client_addr)->sin_addr.s_addr;
                uint16_t port = ((struct sockaddr_in*)&client_addr)->sin_port;
                input_packet.source.reset(new UDPEndPoint(addr, port));
                rv = true;
            }
        }
    }
    else
    {
        if (0 == poll_rv)
        {
            errno = ETIME;
        }
    }

    return rv;
}

bool DiscoveryServerLinux::send_message(OutputPacket&& output_packet)
{
    bool rv = false;
    const UDPEndPoint* destination = static_cast<const UDPEndPoint*>(output_packet.destination.get());
    struct sockaddr_in client_addr;

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = destination->get_port();
    client_addr.sin_addr.s_addr = destination->get_addr();
    ssize_t bytes_sent = sendto(poll_fd_.fd,
                                output_packet.message->get_buf(),
                                output_packet.message->get_len(),
                                0,
                                (struct sockaddr*)&client_addr,
                                sizeof(client_addr));
    if (0 < bytes_sent)
    {
        rv = (size_t(bytes_sent) == output_packet.message->get_len());
    }

    return rv;
}

} // namespace uxr
} // namespace eprosima
