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

#include <uxr/agent/p2p/InternalClientManager.hpp>
#include <uxr/agent/p2p/InternalClient.hpp>

#include <iostream>

namespace eprosima {
namespace uxr {

InternalClientManager& InternalClientManager::instance()
{
    static InternalClientManager manager;
    return manager;
}

void InternalClientManager::set_local_address(
        uint16_t port)
{
    local_client_key_ = port + (uint32_t(0xEA) << 24);
}

void InternalClientManager::create_client(
        Agent& agent,
        const std::array<uint8_t, 4>& ip,
        uint16_t port)
{
    uint32_t remote_client_key = port + (uint32_t(ip[3]) << 16) + (uint32_t(0xEA) << 24);
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = clients_.find(remote_client_key);
    if (clients_.end() == it)
    {
        std::unique_ptr<InternalClient>
                client(new InternalClient(agent, ip, port, remote_client_key, local_client_key_));
        if (client->run())
        {
            clients_.emplace(remote_client_key, std::move(client));
        }
    }
}

void InternalClientManager::delete_clients()
{
    std::lock_guard<std::mutex> lock(mtx_);
    for (auto& c : clients_)
    {
        c.second->stop();
    }
    clients_.clear();
}

InternalClientManager::InternalClientManager() = default;
InternalClientManager::~InternalClientManager() = default;

} // namespace uxr
} // namespace eprosima
