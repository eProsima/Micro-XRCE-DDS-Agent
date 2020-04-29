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

#ifndef UXR_AGENT_P2P_INTERNAL_CLIENT_MANAGER_HPP_
#define UXR_AGENT_P2P_INTERNAL_CLIENT_MANAGER_HPP_

#include <map>
#include <mutex>
#include <memory>

struct uxrAgentAddress;

namespace eprosima {
namespace uxr {

class InternalClient;
class Agent;

class InternalClientManager
{
public:
    static InternalClientManager& instance();

    void set_local_address(
            uint16_t port);

    void create_client(
            Agent& agent,
            const std::array<uint8_t, 4>& ip,
            uint16_t port);

    void delete_clients();

private:
    InternalClientManager();
    ~InternalClientManager();

    InternalClientManager(InternalClientManager&&) = delete;
    InternalClientManager(const InternalClientManager&) = delete;
    InternalClientManager& operator=(InternalClientManager&&) = delete;
    InternalClientManager& operator=(const InternalClientManager&) = delete;

private:
    std::mutex mtx_;
    uint32_t local_client_key_;
    std::map<uint32_t, std::unique_ptr<InternalClient>> clients_;
};

} // namespace uxr
} // namespece eprosima

#endif // UXR_AGENT_P2P_INTERNAL_CLIENT_MANAGER_HPP_
