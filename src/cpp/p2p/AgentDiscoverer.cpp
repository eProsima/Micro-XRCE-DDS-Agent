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

#include <uxr/agent/p2p/AgentDiscoverer.hpp>
#include <uxr/agent/p2p/InternalClientManager.hpp>
#include <uxr/client/client.h>

namespace eprosima {
namespace uxr {

const int timeout = 100; // TODO (julian): get values from configuration.

static void on_agent_found(const uxrAgentAddress* address, int64_t /*timestamp*/, void* /*args*/)
{
    (void) address;
    // Call InternalClientManager.
}

AgentDiscoverer::AgentDiscoverer()
    : running_cond_(false)
    , thread_{}
{
}

bool AgentDiscoverer::run()
{
    bool rv = false;
    if (!running_cond_)
    {
        running_cond_ = true;
        thread_ = std::thread(&AgentDiscoverer::discovery_loop, this);
        rv = true;
    }
    return rv;
}

bool AgentDiscoverer::stop()
{
    bool rv = false;
    if (running_cond_)
    {
        running_cond_ = false;
        if (thread_.joinable())
        {
            thread_.join();
        }
        rv = true;
    }
    return rv;
}

void AgentDiscoverer::discovery_loop()
{
    while (running_cond_)
    {
        uxr_discovery_agents_default(1, timeout, on_agent_found, nullptr);
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
    }
}

} // namespace uxr
} // namespace eprosima
