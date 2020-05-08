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

#ifndef UXR_AGENT_TRANSPORT_P2P_AGENT_DISCOVERER_LINUX_HPP_
#define UXR_AGENT_TRANSPORT_P2P_AGENT_DISCOVERER_LINUX_HPP_

#include <uxr/agent/transport/p2p/AgentDiscoverer.hpp>

#include <poll.h>

namespace eprosima {
namespace uxr {

class AgentDiscovererLinux : public AgentDiscoverer
{
public:
    AgentDiscovererLinux(
            Agent& agent);

    ~AgentDiscovererLinux() = default;

private:
    bool init(
            uint16_t p2p_port) final;

    bool fini() final;

    bool recv_message(
            InputMessagePtr& input_message,
            int timeout) final;

    bool send_message(
            const OutputMessage& output_message) final;

private:
    struct pollfd poll_fd_;
    uint8_t buf_[128];
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_TRANSPORT_P2P_AGENT_DISCOVERER_LINUX_HPP_
