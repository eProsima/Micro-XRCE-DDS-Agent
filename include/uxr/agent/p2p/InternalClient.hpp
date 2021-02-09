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

#ifndef UXR_AGENT_P2P_INTERNAL_CLIENT_HPP_
#define UXR_AGENT_P2P_INTERNAL_CLIENT_HPP_

#include <uxr/client/client.h>
#include <array>
#include <thread>
#include <set>
#include <mutex>
#include <atomic>

namespace eprosima {
namespace uxr {

class Agent;

const uint8_t internal_client_history = 8; // TODO (julian): take from config.

class InternalClient
{
public:
    InternalClient(
            Agent& agent,
            const std::array<uint8_t, 4>& ip,
            uint16_t port,
            uint32_t remote_client_key,
            uint32_t local_client_key);

    ~InternalClient() = default;

    bool run();

    bool stop();

    Agent& get_agent() { return agent_; }

private:
    void set_callback();

    void create_streams();

    void create_domain_entities();

    void create_topic_entities();

    void loop();

    void on_new_domain(int16_t domain);

    void on_new_topic(
            int16_t domain_id,
            const std::string& topic_name);

private:
    Agent& agent_;
    std::array<uint8_t, 4> ip_;
    uint16_t port_;

    /* Domains. */
    std::set<int16_t> domains_;
    std::set<std::pair<int16_t, std::string>> topics_;
    uint16_t topic_counter_;

    /* Transport. */
    uxrUDPTransport transport_;

    /* Client. */
    uint32_t remote_client_key_;
    uint32_t local_client_key_;
    uxrSession session_;
    uint8_t out_buffer_[UXR_CONFIG_UDP_TRANSPORT_MTU * internal_client_history];
    uint8_t in_buffer_[UXR_CONFIG_UDP_TRANSPORT_MTU * internal_client_history];
    uxrStreamId out_stream_id_;
    uxrStreamId in_stream_id_;

    std::atomic<bool> running_cond_;
    std::thread thread_;
    std::mutex mtx_;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_P2P_INTERNAL_CLIENT_HPP_
