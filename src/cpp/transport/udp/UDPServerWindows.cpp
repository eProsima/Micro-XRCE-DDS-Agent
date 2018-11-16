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

#include <uxr/agent/transport/udp/UDPServerWindows.hpp>
#include <string.h>

namespace eprosima {
namespace uxr {

UDPServer::UDPServer(uint16_t port, uint16_t discovery_port)
    : UDPServerBase(port),
      poll_fd_{},
      buffer_{0}
{
    (void) discovery_port;
}

bool UDPServer::init(bool discovery_enabled)
{
    (void) discovery_enabled;

    bool rv = false;

    /* Socker initialization. */
    poll_fd_.fd = socket(PF_INET, SOCK_DGRAM, 0);

    if (INVALID_SOCKET != poll_fd_.fd)
    {
        /* IP and Port setup. */
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_port = htons(port_);
        address.sin_addr.s_addr = INADDR_ANY;
        memset(address.sin_zero, '\0', sizeof(address.sin_zero));
        if (SOCKET_ERROR != bind(poll_fd_.fd, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)))
        {
            /* Poll setup. */
            poll_fd_.events = POLLIN;
            rv = true;
        }
    }

    return rv;
}

bool UDPServer::close()
{
    return (0 == closesocket(poll_fd_.fd));
}

bool UDPServer::recv_message(InputPacket& input_packet, int timeout)
{
    bool rv = false;
    struct sockaddr client_addr;
    int client_addr_len = sizeof(client_addr);

    int poll_rv = WSAPoll(&poll_fd_, 1, timeout);
    if (0 < poll_rv)
    {
        int bytes_received = recvfrom(poll_fd_.fd,
                                      reinterpret_cast<char*>(buffer_),
                                      sizeof(buffer_),
                                      0,
                                      &client_addr,
                                      &client_addr_len);
        if (SOCKET_ERROR != bytes_received)
        {
            input_packet.message.reset(new InputMessage(buffer_, size_t(bytes_received)));
            uint32_t addr = reinterpret_cast<struct sockaddr_in*>(&client_addr)->sin_addr.s_addr;
            uint16_t port = reinterpret_cast<struct sockaddr_in*>(&client_addr)->sin_port;
            input_packet.source.reset(new UDPEndPoint(addr, port));
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

bool UDPServer::send_message(OutputPacket output_packet)
{
    bool rv = false;
    const UDPEndPoint* destination = static_cast<const UDPEndPoint*>(output_packet.destination.get());
    struct sockaddr_in client_addr;

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = destination->get_port();
    client_addr.sin_addr.s_addr = destination->get_addr();
    int bytes_sent = sendto(poll_fd_.fd,
                            reinterpret_cast<char*>(output_packet.message->get_buf()),
                            int(output_packet.message->get_len()),
                            0,
                            reinterpret_cast<struct sockaddr*>(&client_addr),
                            sizeof(client_addr));
    if (SOCKET_ERROR != bytes_sent)
    {
        rv = (size_t(bytes_sent) != output_packet.message->get_len());
    }

    return rv;
}

int UDPServer::get_error()
{
    return WSAGetLastError();
}

} // namespace uxr
} // namespace eprosima
