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

#ifndef UXR_AGENT_TRANSPORT_SESSIONMANAGER_HPP_
#define UXR_AGENT_TRANSPORT_SESSIONMANAGER_HPP_

#include <uxr/agent/logger/Logger.hpp>

#include <memory>
#include <mutex>
#include <map>

namespace eprosima {
namespace uxr {

inline constexpr bool has_session_client_key(uint8_t session_id)
{
    return 128 > session_id;
}

template<typename EndPoint>
class SessionManager
{
public:
    void establish_session(
            const EndPoint& endpoint,
            uint32_t client_key,
            uint8_t session_id);

    void destroy_session(
            const EndPoint& endpoint);

    void destroy_session(
        const uint32_t& client_key);

    bool get_client_key(
            const EndPoint& endpoint,
            uint32_t& client_key);

    bool get_endpoint(
            uint32_t client_key,
            EndPoint& endpoint);

private:
    std::map<EndPoint, uint32_t> endpoint_to_client_map_;
    std::map<uint32_t, EndPoint> client_to_endpoint_map_;
    std::mutex mtx_;
};

template<typename EndPoint>
void SessionManager<EndPoint>::establish_session(
        const EndPoint& endpoint,
        uint32_t client_key,
        uint8_t session_id)
{
    std::lock_guard<std::mutex> lock(mtx_);

    auto it_client = client_to_endpoint_map_.find(client_key);
    if (it_client != client_to_endpoint_map_.end())
    {
        endpoint_to_client_map_.erase(it_client->second);
        it_client->second = endpoint;
        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_GREEN("session re-established"),
            "client_key: 0x{:08X}, address: {}",
            client_key,
            endpoint);
    }
    else
    {
        client_to_endpoint_map_.emplace(client_key, endpoint);
        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_GREEN("session established"),
            "client_key: 0x{:08X}, address: {}",
            client_key,
            endpoint);
    }

    if (!has_session_client_key(session_id))
    {
        auto it_endpoint = endpoint_to_client_map_.find(endpoint);
        if (it_endpoint != endpoint_to_client_map_.end())
        {
            it_endpoint->second = client_key;
        }
        else
        {
            endpoint_to_client_map_.emplace(endpoint, client_key);
        }
    }
}

template<typename EndPoint>
void SessionManager<EndPoint>::destroy_session(
        const EndPoint& endpoint)
{
    std::lock_guard<std::mutex> lock(mtx_);

    auto it = endpoint_to_client_map_.find(endpoint);
    if (it != endpoint_to_client_map_.end())
    {
        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_GREEN("session closed"),
            "client_key: 0x{:08X}, address: {}",
            it->second,
            endpoint);
        client_to_endpoint_map_.erase(it->second);
        endpoint_to_client_map_.erase(it->first);
    }
}

template<typename EndPoint>
void SessionManager<EndPoint>::destroy_session(
        const uint32_t& client_key)
{
    std::lock_guard<std::mutex> lock(mtx_);

    auto it = client_to_endpoint_map_.find(client_key);
    if (it != client_to_endpoint_map_.end())
    {
        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_GREEN("session closed"),
            "client_key: 0x{:08X}, address: {}",
            client_key,
            it->second);
        endpoint_to_client_map_.erase(it->second);
        client_to_endpoint_map_.erase(it->first);
    }
}

template<typename EndPoint>
bool SessionManager<EndPoint>::get_client_key(
        const EndPoint& endpoint,
        uint32_t& client_key)
{
    bool rv = false;
    std::lock_guard<std::mutex> lock(mtx_);

    auto it = endpoint_to_client_map_.find(endpoint);
    if (it != endpoint_to_client_map_.end())
    {
        client_key = it->second;
        rv = true;
    }

    return rv;
}

template<typename EndPoint>
bool SessionManager<EndPoint>::get_endpoint(
        uint32_t client_key,
        EndPoint& endpoint)
{
    bool rv = false;
    std::lock_guard<std::mutex> lock(mtx_);

    auto it = client_to_endpoint_map_.find(client_key);
    if (it != client_to_endpoint_map_.end())
    {
        endpoint = it->second;
        rv = true;
    }

    return rv;
}

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_TRANSPORT_SESSIONMANAGER_HPP_
