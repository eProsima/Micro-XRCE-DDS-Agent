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

#ifndef UXR_AGENT_TRANSPORT_SERIAL_SERVER_BASE_HPP_
#define UXR_AGENT_TRANSPORT_SERIAL_SERVER_BASE_HPP_

#include <uxr/agent/transport/Server.hpp>
#include <uxr/agent/transport/endpoint/SerialEndPoint.hpp>
#include <uxr/agent/transport/serial/SerialProtocol.hpp>

#include <unordered_map>

namespace eprosima {
namespace uxr {

class SerialServerBase : public Server
{
public:
    SerialServerBase(
            uint8_t addr,
            Middleware::Kind middleware_kind);

    virtual ~SerialServerBase() override = default;

    void on_create_client(
            EndPoint* source,
            const dds::xrce::CLIENT_Representation& representation) override;

    void on_delete_client(EndPoint* source) override;

    const dds::xrce::ClientKey get_client_key(EndPoint* source) override;

    std::unique_ptr<EndPoint> get_source(const dds::xrce::ClientKey& client_key) override;

protected:
    uint8_t addr_;

private:
    std::unordered_map<uint8_t, uint32_t> source_to_client_map_;
    std::unordered_map<uint32_t, uint8_t> client_to_source_map_;
    std::mutex clients_mtx_;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_TRANSPORT_SERIAL_SERVER_BASE_HPP_
