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

#ifndef UXR_AGENT_UTILS_TIME_HPP_
#define UXR_AGENT_UTILS_TIME_HPP_

#include <cinttypes>
#include <chrono>

namespace eprosima {
namespace uxr {
namespace time {

inline void get_epoch_time(int32_t& sec, uint32_t& nsec)
{
    auto epoch_time(std::chrono::duration_cast<std::chrono::nanoseconds>
                    (std::chrono::high_resolution_clock::now().time_since_epoch()).count());
    sec = int32_t(epoch_time / std::nano::den);
    nsec = uint32_t(epoch_time % std::nano::den);
}

} // namespace eprosima
} // namespace uxr
} // namespace time

#endif // UXR_AGENT_UTILS_TIME_HPP_
