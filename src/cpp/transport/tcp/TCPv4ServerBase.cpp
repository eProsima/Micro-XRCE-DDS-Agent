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

#include <uxr/agent/transport/tcp/TCPv4ServerBase.hpp>
#include <uxr/agent/utils/Conversion.hpp>
#include <uxr/agent/logger/Logger.hpp>

namespace eprosima {
namespace uxr {

TCPv4ServerBase::TCPv4ServerBase(
        uint16_t agent_port,
        Middleware::Kind middleware_kind)
    : Server(middleware_kind)
    , transport_address_{}
    , source_to_connection_map_{}
    , source_to_client_map_{}
    , client_to_source_map_{}
    , clients_mtx_()
{
    dds::xrce::TransportAddressMedium medium_locator;
    medium_locator.port(agent_port);
    transport_address_.medium_locator(medium_locator);
}

void TCPv4ServerBase::on_create_client(
        EndPoint* source,
        const dds::xrce::CLIENT_Representation& representation)
{
    IPv4EndPoint* endpoint = static_cast<IPv4EndPoint*>(source);
    uint64_t source_id = (uint64_t(endpoint->get_addr()) << 16) | endpoint->get_port();
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

void TCPv4ServerBase::on_delete_client(EndPoint* source)
{
    IPv4EndPoint* endpoint = static_cast<IPv4EndPoint*>(source);
    uint64_t source_id = (uint64_t(endpoint->get_addr()) << 16) | endpoint->get_port();

    /* Update maps. */
    std::lock_guard<std::mutex> lock(clients_mtx_);
    auto it = source_to_client_map_.find(source_id);
    if (it != source_to_client_map_.end())
    {
        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_GREEN("session destroyed"),
            "client_key: 0x{:08X}, address: {}",
            it->second,
            *source);
        client_to_source_map_.erase(it->second);
        source_to_client_map_.erase(it->first);
    }
}

const dds::xrce::ClientKey TCPv4ServerBase::get_client_key(EndPoint* source)
{
    dds::xrce::ClientKey client_key;
    IPv4EndPoint* endpoint = static_cast<IPv4EndPoint*>(source);
    std::lock_guard<std::mutex> lock(clients_mtx_);
    auto it = source_to_client_map_.find((uint64_t(endpoint->get_addr()) << 16) | endpoint->get_port());
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

std::unique_ptr<EndPoint> TCPv4ServerBase::get_source(const dds::xrce::ClientKey& client_key)
{
    std::unique_ptr<EndPoint> source;
    uint32_t client_id = conversion::clientkey_to_raw(client_key);
    std::lock_guard<std::mutex> lock(clients_mtx_);
    auto it = client_to_source_map_.find(client_id);
    if (it != client_to_source_map_.end())
    {
        uint64_t source_id = it->second;
        source.reset(new IPv4EndPoint(uint32_t(source_id >> 16), uint16_t(source_id & 0xFFFF)));
    }
    return source;
}

uint16_t TCPv4ServerBase::read_data(TCPConnection& connection)
{
    uint16_t rv = 0;
    bool exit_flag = false;

    /* State Machine. */
    while(!exit_flag)
    {
        switch (connection.input_buffer.state)
        {
            case TCP_BUFFER_EMPTY:
            {
                connection.input_buffer.position = 0;
                uint8_t size_buf[2];
                uint8_t errcode = 0;
                size_t bytes_received = recv_locking(connection, size_buf, 2, errcode);
                if (0 < bytes_received)
                {
                    connection.input_buffer.msg_size = 0;
                    if (2 == bytes_received)
                    {
                        connection.input_buffer.msg_size = uint16_t((uint16_t(size_buf[1]) << 8) | size_buf[0]);
                        if (connection.input_buffer.msg_size != 0)
                        {
                            connection.input_buffer.state = TCP_SIZE_READ;
                        }
                    }
                    else
                    {
                        connection.input_buffer.msg_size = uint16_t(size_buf[0]);
                        connection.input_buffer.state = TCP_SIZE_INCOMPLETE;
                    }
                }
                else
                {
                    if (0 < errcode)
                    {
                        close_connection(connection);
                    }
                    exit_flag = true;
                }
                break;
            }
            case TCP_SIZE_INCOMPLETE:
            {
                uint8_t size_msb;
                uint8_t errcode = 0;
                size_t bytes_received = recv_locking(connection, &size_msb, 1, errcode);
                if (0 < bytes_received)
                {
                    connection.input_buffer.msg_size = uint16_t((uint16_t(size_msb) << 8) | connection.input_buffer.msg_size);
                    if (connection.input_buffer.msg_size != 0)
                    {
                        connection.input_buffer.state = TCP_SIZE_READ;
                    }
                    else
                    {
                        connection.input_buffer.state = TCP_BUFFER_EMPTY;
                    }
                }
                else
                {
                    if (0 < errcode)
                    {
                        close_connection(connection);
                    }
                    exit_flag = true;
                }
                break;
            }
            case TCP_SIZE_READ:
            {
                connection.input_buffer.buffer.resize(connection.input_buffer.msg_size);
                uint8_t errcode = 0;
                size_t bytes_received = recv_locking(connection,
                                                     connection.input_buffer.buffer.data(),
                                                     connection.input_buffer.buffer.size(),
                                                     errcode);
                if (0 < bytes_received)
                {
                    if (uint16_t(bytes_received) == connection.input_buffer.msg_size)
                    {
                        connection.input_buffer.state = TCP_MESSAGE_AVAILABLE;
                    }
                    else
                    {
                        connection.input_buffer.position = uint16_t(bytes_received);
                        connection.input_buffer.state = TCP_MESSAGE_INCOMPLETE;
                        exit_flag = true;
                    }
                }
                else
                {
                    if (0 < errcode)
                    {
                        close_connection(connection);
                    }
                    exit_flag = true;
                }
                break;
            }
            case TCP_MESSAGE_INCOMPLETE:
            {
                uint8_t errcode = 0;
                size_t bytes_received = recv_locking(connection,
                                                     connection.input_buffer.buffer.data() + connection.input_buffer.position,
                                                     connection.input_buffer.buffer.size() - connection.input_buffer.position,
                                                     errcode);
                if (0 < bytes_received)
                {
                    connection.input_buffer.position += uint16_t(bytes_received);
                    if (connection.input_buffer.position == connection.input_buffer.msg_size)
                    {
                        connection.input_buffer.state = TCP_MESSAGE_AVAILABLE;
                    }
                    else
                    {
                        exit_flag = true;
                    }
                }
                else
                {
                    if (0 < errcode)
                    {
                        close_connection(connection);
                    }
                    exit_flag = true;
                }
                break;
            }
            case TCP_MESSAGE_AVAILABLE:
            {
                rv = connection.input_buffer.msg_size;
                connection.input_buffer.state = TCP_BUFFER_EMPTY;
                exit_flag = true;
                break;
            }
        }
    }

    return rv;
}

} // namespace uxr
} // namespace eprosima
