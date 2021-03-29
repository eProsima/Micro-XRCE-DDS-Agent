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

#ifndef UXR_AGENT_TRANSPORT_ENDPOINT_IPV4_ENDPOINT_HPP_
#define UXR_AGENT_TRANSPORT_ENDPOINT_IPV4_ENDPOINT_HPP_

#include <stdint.h>
#include <iostream>

namespace eprosima {
namespace uxr {

class IPv4EndPoint
{
public:
    IPv4EndPoint() = default;

    IPv4EndPoint(
            uint32_t addr,
            uint16_t port)
        : addr_(addr)
        , port_(port)
    {}

    ~IPv4EndPoint() = default;

    bool operator<(const IPv4EndPoint& other) const
    {
        return (addr_ < other.addr_) || ((addr_ == other.addr_) && (port_ < other.port_));
    }

   friend std::ostream& operator<<(std::ostream& os, const IPv4EndPoint& endpoint)
   {
       os << static_cast<int>(static_cast<uint8_t>(endpoint.addr_)) << "."
          << static_cast<int>(static_cast<uint8_t>(endpoint.addr_ >> 8)) << "."
          << static_cast<int>(static_cast<uint8_t>(endpoint.addr_ >> 16)) << "."
          << static_cast<int>(static_cast<uint8_t>(endpoint.addr_ >> 24)) << ":"
          << endpoint.get_port();
       return os;
   }

    uint32_t get_addr() const { return addr_; }
    uint16_t get_port() const { return port_; }

private:
    uint32_t addr_;
    uint16_t port_;
};


} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_TRANSPORT_ENDPOINT_IPV4_ENDPOINT_HPP_
