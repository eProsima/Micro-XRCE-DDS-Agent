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

#ifndef _UXR_AGENT_TRANSPORT_SERIAL_ENDPOINT_HPP_
#define _UXR_AGENT_TRANSPORT_SERIAL_ENDPOINT_HPP_

#include <stdint.h>

namespace eprosima {
namespace uxr {

class SerialEndPoint
{
public:
    SerialEndPoint() = default;

    SerialEndPoint(
            uint8_t addr)
        : addr_{addr}
    {}

    ~SerialEndPoint() {}

    bool operator<(const SerialEndPoint& other) const
    {
        return (addr_ < other.addr_);
    }

    friend std::ostream& operator<<(std::ostream& os, const SerialEndPoint& endpoint)
    {
        os << static_cast<int>(endpoint.addr_);
        return os;
    }

    uint8_t get_addr() const { return addr_; }

private:
    uint8_t addr_;
};

} // namespace uxr
} // namespace eprosima

#endif //_UXR_AGENT_TRANSPORT_SERIAL_ENDPOINT_HPP_
