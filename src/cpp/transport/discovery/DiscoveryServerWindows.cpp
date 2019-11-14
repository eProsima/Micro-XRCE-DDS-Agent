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

namespace {

template<typename E>
bool update_interfaces(
    uint16_t agent_port,
    std::vector<dds::xrce::TransportAddress>& transport_addresses);

template<>
bool update_interfaces<eprosima::uxr::IPv4EndPoint>(
    uint16_t agent_port,
    std::vector<dds::xrce::TransportAddress>& transport_addresses)
{
    bool rv = false;

    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
    ULONG family = AF_UNSPEC;
    PIP_ADAPTER_ADDRESSES addresses = nullptr;
    PIP_ADAPTER_ADDRESSES current_addr = nullptr;
    PIP_ADAPTER_UNICAST_ADDRESS unicast_addr = nullptr;
    ULONG out_buf_len = 1000; // alloc 1 KB at the begining.
    DWORD rv_get_adapters;

    addresses = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(malloc(out_buf_len));
    if (nullptr == addresses)
    {
        return false;
    }

    rv_get_adapters = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, addresses, &out_buf_len);
    if (ERROR_BUFFER_OVERFLOW == rv_get_adapters)
    {
        addresses = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(realloc(addresses, out_buf_len));
        if (nullptr == addresses)
        {
            return false;
        }
        rv_get_adapters = GetAdaptersAddresses(family, flags, nullptr, addresses, &out_buf_len);
    }

    if (NO_ERROR == rv_get_adapters)
    {
        transport_addresses.clear();
        current_addr = addresses;
        while (current_addr)
        {
            unicast_addr = current_addr->FirstUnicastAddress;
            while (unicast_addr)
            {
                if (AF_INET == unicast_addr->Address.lpSockaddr->sa_family)
                {
                    dds::xrce::TransportAddressMedium medium_locator;
                    medium_locator.port(agent_port);
                    medium_locator.address(
                        {uint8_t(unicast_addr->Address.lpSockaddr->sa_data[2]),
                         uint8_t(unicast_addr->Address.lpSockaddr->sa_data[3]),
                         uint8_t(unicast_addr->Address.lpSockaddr->sa_data[4]),
                         uint8_t(unicast_addr->Address.lpSockaddr->sa_data[5])});
                    transport_addresses_.emplace_back();
                    transport_addresses_.back().medium_locator(medium_locator);
                }
                unicast_addr = unicast_addr->Next;
            }
            current_addr = current_addr->Next;
        }
        rv = true;
    }
    free(addresses);

    return rv;
}

template<>
bool update_interfaces<eprosima::uxr::IPv6EndPoint>(
    uint16_t agent_port,
    std::vector<dds::xrce::TransportAddress>& transport_addresses)
{
    bool rv = false;

    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
    ULONG family = AF_UNSPEC;
    PIP_ADAPTER_ADDRESSES addresses = nullptr;
    PIP_ADAPTER_ADDRESSES current_addr = nullptr;
    PIP_ADAPTER_UNICAST_ADDRESS unicast_addr = nullptr;
    ULONG out_buf_len = 1000; // alloc 1 KB at the begining.
    DWORD rv_get_adapters;

    addresses = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(malloc(out_buf_len));
    if (nullptr == addresses)
    {
        return false;
    }

    rv_get_adapters = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, addresses, &out_buf_len);
    if (ERROR_BUFFER_OVERFLOW == rv_get_adapters)
    {
        addresses = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(realloc(addresses, out_buf_len));
        if (nullptr == addresses)
        {
            return false;
        }
        rv_get_adapters = GetAdaptersAddresses(family, flags, nullptr, addresses, &out_buf_len);
    }

    if (NO_ERROR == rv_get_adapters)
    {
        transport_addresses.clear();
        current_addr = addresses;
        while (current_addr)
        {
            unicast_addr = current_addr->FirstUnicastAddress;
            while (unicast_addr)
            {
                if (AF_INET6 == unicast_addr->Address.lpSockaddr->sa_family)
                {
                    dds::xrce::TransportAddressLarge large_locator;
                    large_locator.port(agent_port);
                    struct sockaddr_in6* addr = reinterpret_cast<sockaddr_in6*>(unicast_addr->Address.lpSockaddr);
                    large_locator.address(
                        {addr->sin6_addr.s6_addr[0],
                         addr->sin6_addr.s6_addr[1],
                         addr->sin6_addr.s6_addr[2],
                         addr->sin6_addr.s6_addr[3],
                         addr->sin6_addr.s6_addr[4],
                         addr->sin6_addr.s6_addr[5],
                         addr->sin6_addr.s6_addr[6],
                         addr->sin6_addr.s6_addr[7],
                         addr->sin6_addr.s6_addr[8],
                         addr->sin6_addr.s6_addr[9],
                         addr->sin6_addr.s6_addr[10],
                         addr->sin6_addr.s6_addr[11],
                         addr->sin6_addr.s6_addr[12],
                         addr->sin6_addr.s6_addr[13],
                         addr->sin6_addr.s6_addr[14],
                         addr->sin6_addr.s6_addr[15]});
                    transport_addresses.emplace_back();
                    transport_addresses.back().large_locator(large_locator);
                }
                unicast_addr = unicast_addr->Next;
            }
            current_addr = current_addr->Next;
        }
        rv = true;
    }
    free(addresses);

    return rv;
}

} // anonymous namespace

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

    /* Get interfaces. */
    if (!update_interfaces())
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
            UXR_AGENT_LOG_INFO(
                UXR_DECORATE_GREEN("running..."),
                "Port: {}",
                discovery_port);
            rv = true;
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
    }

    return rv;
}

template<typename EndPoint>
bool DiscoveryServerWindows<EndPoint>::update_interfaces()
{
    return ::update_interfaces<EndPoint>(
        this->agent_port_,
        this->transport_addresses_);
}

template class DiscoveryServerWindows<IPv4EndPoint>;
template class DiscoveryServerWindows<IPv6EndPoint>;

} // namespace uxr
} // namespace eprosima
