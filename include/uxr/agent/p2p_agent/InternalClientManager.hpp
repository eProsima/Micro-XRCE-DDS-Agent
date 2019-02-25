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


#ifndef UXR_AGENT_P2P_AGENT_INTERNAL_CLIENT_MANAGER_HPP_
#define UXR_AGENT_P2P_AGENT_INTERNAL_CLIENT_MANAGER_HPP_

#include <uxr/agent/middleware/ced/CedEntities.hpp> //For TopicManager

#include <string>
#include <utility>
#include <map>

namespace eprosima {
namespace uxr {

class InternalClient;

class InternalClientManager
{
public:
    InternalClientManager();
    virtual ~InternalClientManager();

    void create_client(const std::string& ip, int port);
    bool has_client_listening_from(const std::string& ip, int port);
    void topic_register_event_change(/* event_type and topic */);

private:
    struct InternalClientKey
    {
        std::string ip_;
        int port_;

        InternalClientKey(const std::string& ip, int port)
           : ip_(ip)
           , port_(port)
        {}

        bool operator < (const InternalClientKey& other) const
        {
           return ip_ < other.ip_ && port_ < other.port_;
        }
    };

    std::map<InternalClientKey, InternalClient> internal_clients_;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_P2P_AGENT_INTERNAL_CLIENT_MANAGER_HPP_
