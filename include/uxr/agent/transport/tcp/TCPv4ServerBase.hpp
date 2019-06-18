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

#ifndef UXR_AGENT_TRANSPORT_TCPv4_SERVER_BASE_HPP_
#define UXR_AGENT_TRANSPORT_TCPv4_SERVER_BASE_HPP_

#include <uxr/agent/transport/Server.hpp>
#include <uxr/agent/transport/tcp/TCPConnection.hpp>
#include <uxr/agent/transport/endpoint/IPv4EndPoint.hpp>

#include <unordered_map>

namespace eprosima {
namespace uxr {

class TCPConnection;

class TCPv4ServerBase : public Server
{
public:
    TCPv4ServerBase(
            uint16_t agent_port,
            Middleware::Kind middleware_kind);

    virtual ~TCPv4ServerBase() override = default;

    void on_create_client(
            EndPoint* source,
            const dds::xrce::CLIENT_Representation& representation) override;

    void on_delete_client(EndPoint* source) override;

    const dds::xrce::ClientKey get_client_key(EndPoint *source) override;

    std::unique_ptr<EndPoint> get_source(const dds::xrce::ClientKey& client_key) override;

private:
    virtual bool close_connection(TCPConnection& connection) = 0;

    virtual size_t recv_locking(
            TCPConnection& connection,
            uint8_t* buffer,
            size_t len,
            uint8_t& errcode) = 0;

    virtual size_t send_locking(
            TCPConnection& connection,
            uint8_t* buffer,
            size_t len,
            uint8_t& errcode) = 0;

protected:
    uint16_t read_data(TCPConnection& connection);

protected:
    dds::xrce::TransportAddress transport_address_;
    std::unordered_map<uint64_t, uint32_t> source_to_connection_map_;
    std::unordered_map<uint64_t, uint32_t> source_to_client_map_;
    std::unordered_map<uint32_t, uint64_t> client_to_source_map_;
    std::mutex clients_mtx_;

};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_TRANSPORT_TCPv4_SERVER_BASE_HPP_
