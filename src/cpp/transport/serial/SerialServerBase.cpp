// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <uxr/agent/transport/serial/SerialServerBase.hpp>
#include <uxr/agent/utils/Conversion.hpp>
#include <uxr/agent/logger/Logger.hpp>

namespace eprosima {
namespace uxr {

SerialServerBase::SerialServerBase(
        uint8_t addr,
        Middleware::Kind middleware_kind)
    : Server(middleware_kind)
    , addr_(addr)
    , source_to_client_map_{}
    , client_to_source_map_{}
{}

void SerialServerBase::on_create_client(
        EndPoint* source,
        const dds::xrce::CLIENT_Representation& representation)
{
    SerialEndPoint* endpoint = static_cast<SerialEndPoint*>(source);
    uint8_t source_id = endpoint->get_addr();
    const dds::xrce::ClientKey& client_key = representation.client_key();
    uint32_t client_id = conversion::clientkey_to_raw(client_key);

    /* Update source for the client. */
    std::lock_guard<std::mutex> lock(clients_mtx_);
    auto it_client = client_to_source_map_.find(client_id);
    if (it_client != client_to_source_map_.end())
    {
        source_to_client_map_.erase(it_client->second);
        it_client->second = source_id;
    }
    else
    {
        client_to_source_map_.insert(std::make_pair(client_id, source_id));
        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_GREEN("session established"),
            "client_key: 0x{:08X}, address: {}",
            client_id,
            *source);

    }

    /* Update client for the source. */
    if (127 < representation.session_id())
    {
        auto it_source = source_to_client_map_.find(source_id);
        if (it_source != source_to_client_map_.end())
        {
            it_source->second = client_id;
            UXR_AGENT_LOG_INFO(
                UXR_DECORATE_WHITE("address updated"),
                "client_key: 0x{:08X}, address: {}",
                client_id,
                *source);
        }
        else
        {
           source_to_client_map_.insert(std::make_pair(source_id, client_id));
        }
    }
}

void SerialServerBase::on_delete_client(EndPoint* source)
{
    SerialEndPoint* endpoint = static_cast<SerialEndPoint*>(source);
    uint8_t source_id = endpoint->get_addr();

    /* Update maps. */
    std::lock_guard<std::mutex> lock(clients_mtx_);
    auto it = source_to_client_map_.find(source_id);
    if (it != source_to_client_map_.end())
    {
        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_WHITE("session destroyed"),
            "client_key: 0x{:08X}, address: {}",
            it->second,
            *source);
        client_to_source_map_.erase(it->second);
        source_to_client_map_.erase(it->first);
    }
}

const dds::xrce::ClientKey SerialServerBase::get_client_key(EndPoint* source)
{
    dds::xrce::ClientKey client_key;
    SerialEndPoint* endpoint = static_cast<SerialEndPoint*>(source);
    std::lock_guard<std::mutex> lock(clients_mtx_);
    auto it = source_to_client_map_.find(endpoint->get_addr());
    if (it != source_to_client_map_.end())
    {
        client_key = conversion::raw_to_clientkey(it->second);
    }
    else
    {
        client_key = dds::xrce::CLIENTKEY_INVALID;
    }
    return client_key;
}

std::unique_ptr<EndPoint> SerialServerBase::get_source(const dds::xrce::ClientKey& client_key)
{
    std::unique_ptr<EndPoint> source;
    uint32_t client_id = conversion::clientkey_to_raw(client_key);
    std::lock_guard<std::mutex> lock(clients_mtx_);
    auto it = client_to_source_map_.find(client_id);
    if (it != client_to_source_map_.end())
    {
        source.reset(new SerialEndPoint(it->second));
    }
    return source;
}

} // namespace uxr
} // namespace eprosima
