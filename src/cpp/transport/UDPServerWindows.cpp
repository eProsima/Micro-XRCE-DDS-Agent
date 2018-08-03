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

#include <micrortps/agent/transport/UDPServerWindows.hpp>
#include <string.h>

namespace eprosima {
namespace micrortps {

UDPServer::UDPServer(uint16_t port)
    : port_(port),
      poll_fd_{},
      buffer_{0},
      source_to_client_map_{},
      client_to_source_map_{}
{}

void UDPServer::on_create_client(EndPoint* source, const dds::xrce::ClientKey& client_key)
{
    UDPEndPoint* endpoint = static_cast<UDPEndPoint*>(source);
    uint64_t source_id = ((uint64_t)endpoint->get_addr() << 16) | endpoint->get_port();
    uint32_t client_id = client_key.at(0) + (client_key.at(1) << 8) + (client_key.at(2) << 16) + (client_key.at(3) <<24);

    /* Update maps. */
    auto it_client = client_to_source_map_.find(client_id);
    if (it_client != client_to_source_map_.end())
    {
        source_to_client_map_.erase(it_client->second);
        it_client->second = source_id;
    }
    else
    {
        client_to_source_map_.insert(std::make_pair(client_id, source_id));
    }

    auto it_source = source_to_client_map_.find(source_id);
    if (it_source != source_to_client_map_.end())
    {
        it_source->second = client_id;
    }
    else
    {
        source_to_client_map_.insert(std::make_pair(source_id, client_id));
    }
}

void UDPServer::on_delete_client(EndPoint* source)
{
    UDPEndPoint* endpoint = static_cast<UDPEndPoint*>(source);
    uint64_t source_id = (endpoint->get_addr() << 16) | endpoint->get_port();

    /* Update maps. */
    auto it = source_to_client_map_.find(source_id);
    if (it != source_to_client_map_.end())
    {
        client_to_source_map_.erase(it->second);
        source_to_client_map_.erase(it->first);
    }
}

const dds::xrce::ClientKey UDPServer::get_client_key(EndPoint* source)
{
    dds::xrce::ClientKey client_key;
    UDPEndPoint* endpoint = static_cast<UDPEndPoint*>(source);
    auto it = source_to_client_map_.find(((uint64_t)endpoint->get_addr() << 16) | endpoint->get_port());
    if (it != source_to_client_map_.end())
    {
        client_key.at(0) = static_cast<uint8_t>(it->second & 0x000000FF);
        client_key.at(1) = static_cast<uint8_t>((it->second & 0x0000FF00) >> 8);
        client_key.at(2) = static_cast<uint8_t>((it->second & 0x00FF0000) >> 16);
        client_key.at(3) = static_cast<uint8_t>((it->second & 0xFF000000) >> 24);
    }
    else
    {
        client_key = dds::xrce::CLIENTKEY_INVALID;
    }
    return client_key;
}

std::unique_ptr<EndPoint> UDPServer::get_source(const dds::xrce::ClientKey& client_key)
{
    std::unique_ptr<EndPoint> source;
    uint32_t client_id = client_key.at(0) + (client_key.at(1) << 8) + (client_key.at(2) << 16) + (client_key.at(3) <<24);
    auto it = client_to_source_map_.find(client_id);
    if (it != client_to_source_map_.end())
    {
        uint64_t source_id = it->second;
        source.reset(new UDPEndPoint(static_cast<uint32_t>(source_id >> 16), static_cast<uint16_t>(source_id & 0xFFFF)));
    }
    return source;
}

bool UDPServer::init()
{
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
        if (SOCKET_ERROR != bind(poll_fd_.fd, (struct sockaddr*)&address, sizeof(address)))
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
    bool rv = true;
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
            input_packet.message.reset(new InputMessage(buffer_, static_cast<size_t>(bytes_received)));
            uint32_t addr = ((struct sockaddr_in*)&client_addr)->sin_addr.s_addr;
            uint16_t port = ((struct sockaddr_in*)&client_addr)->sin_port;
            input_packet.source.reset(new UDPEndPoint(addr, port));
        }
    }
    else if (0 == poll_rv)
    {
        WSASetLastError(WAIT_TIMEOUT);
        rv = false;
    }
    else
    {
        rv = false;
    }

    return rv;
}

bool UDPServer::send_message(OutputPacket output_packet)
{
    bool rv = true;
    const UDPEndPoint* destination = static_cast<const UDPEndPoint*>(output_packet.destination.get());
    struct sockaddr_in client_addr;

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = destination->get_port();
    client_addr.sin_addr.s_addr = destination->get_addr();
    int bytes_sent = sendto(poll_fd_.fd,
                            reinterpret_cast<char*>(output_packet.message->get_buf()),
                            static_cast<int>(output_packet.message->get_len()),
                            0,
                            (struct sockaddr*)&client_addr,
                            sizeof(client_addr));
    if (SOCKET_ERROR != bytes_sent)
    {
        if ((size_t)bytes_sent != output_packet.message->get_len())
        {
            rv = false;
        }
    }
    else
    {
        rv = false;
    }

    return rv;
}

int UDPServer::get_error()
{
    return WSAGetLastError();
}

} // namespace micrortps
} // namespace eprosima
