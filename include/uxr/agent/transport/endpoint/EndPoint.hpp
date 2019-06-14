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

#ifndef UXR_AGENT_TRANSPORT_ENDPOINT_ENDPOINT_HPP_
#define UXR_AGENT_TRANSPORT_ENDPOINT_ENDPOINT_HPP_

#include <iostream>

namespace eprosima {
namespace uxr {

class EndPoint
{
public:
    EndPoint() = default;
    virtual ~EndPoint() = default;

    virtual std::ostream& print(std::ostream& os) const = 0;
};

inline std::ostream& operator<<(std::ostream& os, const EndPoint& endpoint)
{
    return endpoint.print(os);
}

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_TRANSPORT_ENDPOINT_ENDPOINT_HPP_
