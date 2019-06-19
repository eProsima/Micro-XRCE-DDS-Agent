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

#ifndef UXR_AGENT_TRANSPORT_SESSION_MANAGER_HPP_
#define UXR_AGENT_TRANSPORT_SESSION_MANAGER_HPP_

#include <uxr/agent/transport/endpoint/EndPoint.hpp>
#include <uxr/agent/logger/Logger.hpp>

#include <memory>
#include <mutex>
#include <map>

namespace eprosima {
namespace uxr {

template<typename T>
class SessionManager
{
public:
    void on_create_client(
            EndPoint* raw_endpoint,
            uint32_t client_key,
            uint8_t session_id);

    void on_delete_client(
            EndPoint* raw_endpoint);

    uint32_t get_client_key(
            EndPoint* raw_endpoint);

    std::unique_ptr<EndPoint> get_endpoint(
            uint32_t client_key);

private:
    std::map<T, uint32_t> endpoint_to_client_map_;
    std::map<uint32_t, T> client_to_endpoint_map_;
    std::mutex mtx_;
};

template<typename T>
void SessionManager<T>::on_create_client(
        EndPoint* raw_endpoint,
        uint32_t client_key,
        uint8_t session_id)
{
    T* endpoint = static_cast<T*>(raw_endpoint);
    std::lock_guard<std::mutex> lock(mtx_);

    auto it_client = client_to_endpoint_map_.find(client_key);
    if (it_client != client_to_endpoint_map_.end())
    {
        endpoint_to_client_map_.erase(it_client->second);
        it_client->second = *endpoint;
    }
    else
    {
        client_to_endpoint_map_.insert(std::make_pair(client_key, *endpoint));
        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_GREEN("session established"),
            "client_key: 0x{:08}, address: {}",
            client_key,
            *endpoint);
    }

    if (127 < session_id)
    {
        auto it_endpoint = endpoint_to_client_map_.find(*endpoint);
        if (it_endpoint != endpoint_to_client_map_.end())
        {
            it_endpoint->second = client_key;
            UXR_AGENT_LOG_INFO(
                UXR_DECORATE_GREEN("address updated"),
                "client_ket: 0x{:08}, address: {}",
                client_key,
                *endpoint);
        }
        else
        {
            endpoint_to_client_map_.insert(std::make_pair(*endpoint, client_key));
        }
    }
}

template<typename T>
void SessionManager<T>::on_delete_client(
        EndPoint* raw_endpoint)
{
    T endpoint = static_cast<T*>(raw_endpoint);
    std::lock_guard<std::mutex> lock(mtx_);

    auto it = endpoint_to_client_map_.find(*endpoint);
    if (it != endpoint_to_client_map_.end())
    {
        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_GREEN("session closed"),
            "client_key: 0x{:08X}, address: {}",
            it->second,
             *endpoint);
        client_to_endpoint_map_.erase(it->second);
        endpoint_to_client_map_.erase(it->first);
    }
}

template<typename T>
uint32_t SessionManager<T>::get_client_key(
        EndPoint* raw_endpoint)
{
    uint32_t client_key = 0;
    T* endpoint = static_cast<T*>(raw_endpoint);
    std::lock_guard<std::mutex> lock(mtx_);

    auto it = endpoint_to_client_map_.find(*endpoint);
    if (it != endpoint_to_client_map_.end())
    {
        client_key = it->second;
    }

    return client_key;
}

template<typename T>
std::unique_ptr<EndPoint> SessionManager<T>::get_endpoint(
        uint32_t client_key)
{
    std::unique_ptr<EndPoint> raw_endpoint;
    std::lock_guard<std::mutex> lock(mtx_);

    auto it = client_to_endpoint_map_.find(client_key);
    if (it != client_to_endpoint_map_.end())
    {
        raw_endpoint.reset(new T(it->second));
    }

    return raw_endpoint;
}

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_TRANSPORT_SESSION_MANAGER_HPP_
