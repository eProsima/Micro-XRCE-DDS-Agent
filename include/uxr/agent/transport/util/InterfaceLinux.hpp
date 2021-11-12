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

#ifndef UXR_AGENT_TRANSPORT_UTIL_INTERFACELINUX_HPP_
#define UXR_AGENT_TRANSPORT_UTIL_INTERFACELINUX_HPP_

#include <uxr/agent/transport/endpoint/IPv4EndPoint.hpp>
#include <uxr/agent/transport/endpoint/IPv6EndPoint.hpp>
#include <uxr/agent/logger/Logger.hpp>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ifaddrs.h>

namespace eprosima {
namespace uxr {
namespace util {

template<typename E>
void get_transport_interfaces(
    uint16_t agent_port,
    std::vector<dds::xrce::TransportAddress>& transport_addresses);

template<>
inline
void get_transport_interfaces<IPv4EndPoint>(
    uint16_t agent_port,
    std::vector<dds::xrce::TransportAddress>& transport_addresses)
{
    struct ifaddrs* ifaddr;
    struct ifaddrs* ptr;

    transport_addresses.clear();
    if (-1 != getifaddrs(&ifaddr))
    {
        for (ptr = ifaddr; ptr != nullptr; ptr = ptr->ifa_next)
        {
            if (NULL != ptr->ifa_addr && AF_INET == ptr->ifa_addr->sa_family)
            {
                dds::xrce::TransportAddressMedium medium_locator;
                medium_locator.port(agent_port);
                medium_locator.address(
                    {uint8_t(ptr->ifa_addr->sa_data[2]),
                     uint8_t(ptr->ifa_addr->sa_data[3]),
                     uint8_t(ptr->ifa_addr->sa_data[4]),
                     uint8_t(ptr->ifa_addr->sa_data[5])});
                transport_addresses.emplace_back();
                transport_addresses.back().medium_locator(medium_locator);

                UXR_AGENT_LOG_TRACE(
                    UXR_DECORATE_WHITE("interface found"),
                    "address: {}",
                    transport_addresses.back()
                );
            }
        }
    }
    freeifaddrs(ifaddr);
}

template<>
inline
void get_transport_interfaces<IPv6EndPoint>(
    uint16_t agent_port,
    std::vector<dds::xrce::TransportAddress>& transport_addresses)
{
    struct ifaddrs* ifaddr;
    struct ifaddrs* ptr;

    transport_addresses.clear();
    if (-1 != getifaddrs(&ifaddr))
    {
        for (ptr = ifaddr; ptr != nullptr; ptr = ptr->ifa_next)
        {
            if (NULL != ptr->ifa_addr && AF_INET6 == ptr->ifa_addr->sa_family)
            {
                dds::xrce::TransportAddressLarge large_locator;
                large_locator.port(agent_port);
                struct sockaddr_in6* addr = reinterpret_cast<sockaddr_in6*>(ptr->ifa_addr);
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

                UXR_AGENT_LOG_TRACE(
                    UXR_DECORATE_WHITE("interface found"),
                    "address: {}",
                    transport_addresses.back()
                );
            }
        }
    }
    freeifaddrs(ifaddr);
}

} // namespace util
} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_TRANSPORT_UTIL_INTERFACELINUX_HPP_