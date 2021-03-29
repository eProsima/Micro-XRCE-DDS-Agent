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

#include <stdint.h>
#include <iostream>
#include <iomanip>
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
        os << std::setfill('0') << std::setw(2) << std::hex << int(endpoint.addr_.at(0))
           << std::setfill('0') << std::setw(2) << std::hex << int(endpoint.addr_.at(1))
           << ":"
           << std::setfill('0') << std::setw(2) << std::hex << int(endpoint.addr_.at(2))
           << std::setfill('0') << std::setw(2) << std::hex << int(endpoint.addr_.at(3))
           << ":"
           << std::setfill('0') << std::setw(2) << std::hex << int(endpoint.addr_.at(4))
           << std::setfill('0') << std::setw(2) << std::hex << int(endpoint.addr_.at(5))
           << ":"
           << std::setfill('0') << std::setw(2) << std::hex << int(endpoint.addr_.at(6))
           << std::setfill('0') << std::setw(2) << std::hex << int(endpoint.addr_.at(7))
           << ":"
           << std::setfill('0') << std::setw(2) << std::hex << int(endpoint.addr_.at(8))
           << std::setfill('0') << std::setw(2) << std::hex << int(endpoint.addr_.at(9))
           << ":"
           << std::setfill('0') << std::setw(2) << std::hex << int(endpoint.addr_.at(10))
           << std::setfill('0') << std::setw(2) << std::hex << int(endpoint.addr_.at(11))
           << ":"
           << std::setfill('0') << std::setw(2) << std::hex << int(endpoint.addr_.at(12))
           << std::setfill('0') << std::setw(2) << std::hex << int(endpoint.addr_.at(13))
           << ":"
           << std::setfill('0') << std::setw(2) << std::hex << int(endpoint.addr_.at(14))
           << std::setfill('0') << std::setw(2) << std::hex << int(endpoint.addr_.at(15))
           << ":" << endpoint.port_;

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
