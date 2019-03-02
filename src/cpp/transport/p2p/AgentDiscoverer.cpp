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

#include <uxr/agent/transport/p2p/AgentDiscoverer.hpp>

namespace eprosima {
namespace uxr {

AgentDiscoverer::AgentDiscoverer()
    : running_cond_{false}
{
}

bool AgentDiscoverer::run(uint16_t discovery_port)
{
    if (running_cond_ || !init(discovery_port))
    {
        return false;
    }

    /* Init thread. */
    running_cond_ = true;
    thread_ = std::thread(&AgentDiscoverer::loop, this);

    return true;
}

bool AgentDiscoverer::stop()
{
    /* Stop thread. */
    running_cond_ = false;
    if (thread_.joinable())
    {
        thread_.join();
    }
    return close();
}

void AgentDiscoverer::loop()
{
}

} // namespace uxr
} // namespace eprosima
