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

#ifndef _UXR_AGENT_TRANSPORT_CAN_ENDPOINT_HPP_
#define _UXR_AGENT_TRANSPORT_CAN_ENDPOINT_HPP_

#include <stdint.h>

namespace eprosima {
namespace uxr {

class CanEndPoint
{
public:
    CanEndPoint() = default;

    CanEndPoint(
            uint32_t can_id)
        : can_id_{can_id}
    {}

    ~CanEndPoint() {}

    bool operator<(const CanEndPoint& other) const
    {
        return (can_id_ < other.can_id_);
    }

    friend std::ostream& operator<<(std::ostream& os, const CanEndPoint& endpoint)
    {
        os << static_cast<int>(endpoint.can_id_);
        return os;
    }

    uint32_t get_can_id() const { return can_id_; }

private:
    uint32_t can_id_;
};

} // namespace uxr
} // namespace eprosima

#endif //_UXR_AGENT_TRANSPORT_CAN_ENDPOINT_HPP_
