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

#ifndef UXR_AGENT_TRANSPORT_ENDPOINT_IPV6_ENDPOINT_HPP_
#define UXR_AGENT_TRANSPORT_ENDPOINT_IPV6_ENDPOINT_HPP_

#include <uxr/agent/transport/endpoint/EndPoint.hpp>

#include <stdint.h>
#include <iostream>
#include <array>

namespace eprosima {
namespace uxr {

class IPv6EndPoint
{
public:
    IPv6EndPoint() = default;

    IPv6EndPoint(
            const std::array<uint8_t, 16>& addr,
            uint16_t port)
        : addr_(addr)
        , port_(port)
    {}

    ~IPv6EndPoint() = default;

    bool operator<(const IPv6EndPoint& other) const
    {
        return (addr_ < other.addr_) || ((addr_ == other.addr_) && (port_ < other.port_));
    }

    friend std::ostream& operator<<(std::ostream& os, const IPv6EndPoint& endpoint)
    {
        char ip_str[40];
        sprintf(ip_str, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
            int(endpoint.addr_.at(0)), int(endpoint.addr_.at(1)),
            int(endpoint.addr_.at(2)), int(endpoint.addr_.at(3)),
            int(endpoint.addr_.at(4)), int(endpoint.addr_.at(5)),
            int(endpoint.addr_.at(6)), int(endpoint.addr_.at(7)),
            int(endpoint.addr_.at(8)), int(endpoint.addr_.at(9)),
            int(endpoint.addr_.at(10)), int(endpoint.addr_.at(11)),
            int(endpoint.addr_.at(12)), int(endpoint.addr_.at(13)),
            int(endpoint.addr_.at(14)), int(endpoint.addr_.at(15)));
        os << ip_str << ":" << endpoint.port_;
        return os;
    }

    const std::array<uint8_t, 16>& get_addr() const { return addr_; }
    uint16_t get_port() const { return port_; }

private:
    std::array<uint8_t, 16> addr_;
    uint16_t port_;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_TRANSPORT_ENDPOINT_IPV6_ENDPOINT_HPP_
