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


#ifndef UXR_AGENT_P2P_AGENT_DISCOVERY_CLIENT_HPP_
#define UXR_AGENT_P2P_AGENT_DISCOVERY_CLIENT_HPP_

#include <string>
#include <vector>
#include <functional>

struct uxrAgentAddress;

namespace eprosima {
namespace uxr {

typedef std::function<void(const std::string& ip, int port)> AgentDiscoveryCallback;

class DiscoveryClient
{
public:
    DiscoveryClient(AgentDiscoveryCallback callback);
    virtual ~DiscoveryClient();

    void discover(int timeout);
    void add_discovery_address(const std::string& ip, int port);

private:
    AgentDiscoveryCallback agent_discovery_callback_;
    std::vector<std::pair<std::string, int>> discovery_addresses_;

    static void on_agent_found(const uxrAgentAddress* address, int64_t timestamp, void* args);
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_P2P_AGENT_DISCOVERY_CLIENT_HPP_
