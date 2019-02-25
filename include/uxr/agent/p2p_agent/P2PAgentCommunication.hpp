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


#ifndef UXR_AGENT_P2P_AGENT_P2P_AGENT_COMMUNICATION_HPP_
#define UXR_AGENT_P2P_AGENT_P2P_AGENT_COMMUNICATION_HPP_

#include <uxr/agent/p2p_agent/InternalClientManager.hpp>
#include <uxr/agent/p2p_agent/DiscoveryClient.hpp>

#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <thread>

#define UXR_DEFAULT_DISCOVERY_PERIOD 100 //ms

namespace eprosima {
namespace uxr {

class P2PAgentCommunication
{
public:
    enum Mode
    {
        DISABLE = 0,
        PUBLISH = 1 << 0,
        SUBSCRIBE = 1 << 1
    };

    P2PAgentCommunication(Mode mode);
    virtual ~P2PAgentCommunication();

    void run();

    void set_mode(Mode mode) { mode_ = mode; }
    void set_discovery_period(int ms) { period_ = ms; }

private:
    Mode mode_;
    int period_;
    std::thread discovery_thread_;
    DiscoveryClient discovery_client_;
    InternalClientManager client_manager_;

    void on_discover_agent(const std::string& ip, int port);
    void run_discovery();
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_P2P_AGENT_P2P_AGENT_COMMUNICATION_HPP_
