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
        const std::array<uint8_t, 4>& ip,
        uint16_t port)
{
    internal_client_key_ = port + (uint32_t(ip[3]) << 16) + (uint32_t(0xEA) << 24);
}

void InternalClientManager::create_client(
        const std::array<uint8_t, 4>& ip,
        uint16_t port)
{
    uint64_t key =
            (uint64_t(ip[0]) << 40) +
            (uint64_t(ip[1]) << 32) +
            (uint64_t(ip[2]) << 24) +
            (uint64_t(ip[3]) << 16) +
            port;
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = clients_.find(key);
    if (clients_.end() == it)
    {
        std::unique_ptr<InternalClient> client(new InternalClient(ip, port, internal_client_key_));
        if (client->run())
        {
            clients_.emplace(key, std::move(client));
        }
    }
}

InternalClientManager::InternalClientManager() = default;
InternalClientManager::~InternalClientManager() = default;

} // namespace uxr
} // namespace eprosima
