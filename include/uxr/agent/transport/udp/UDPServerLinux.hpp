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

#ifndef _UXR_AGENT_TRANSPORT_UDP_SERVER_HPP_
#define _UXR_AGENT_TRANSPORT_UDP_SERVER_HPP_

#include <uxr/agent/transport/Server.hpp>
#include <uxr/agent/transport/udp/UDPEndPoint.hpp>
#include <uxr/agent/transport/discovery/DiscoveryLinux.hpp>
#include <uxr/agent/config.hpp>
#include <cstdint>
#include <cstddef>
#include <sys/poll.h>
#include <unordered_map>

namespace eprosima {
namespace uxr {

class UDPServer : public Server
{
public:
    UDPServer(uint16_t port);
    ~UDPServer() = default;

    virtual void on_create_client(EndPoint* source, const dds::xrce::ClientKey& client_key) override;
    virtual void on_delete_client(EndPoint* source) override;
    virtual const dds::xrce::ClientKey get_client_key(EndPoint* source) override;
    virtual std::unique_ptr<EndPoint> get_source(const dds::xrce::ClientKey& client_key) override;

private:
    virtual bool init() override;
    virtual bool close() override;
    virtual bool recv_message(InputPacket& input_packet, int timeout) override;
    virtual bool send_message(OutputPacket output_packet) override;
    virtual int get_error() override;
    virtual bool recv_discovery_request(InputPacket& input_packet,
                                        int timeout,
                                        dds::xrce::TransportAddress& address) override;
    virtual bool send_discovery_response(OutputPacket output_packet) override;

private:
    uint16_t port_;
    struct pollfd poll_fd_;
    uint8_t buffer_[UDP_TRANSPORT_MTU];
    std::unordered_map<uint64_t, uint32_t> source_to_client_map_;
    std::unordered_map<uint32_t, uint64_t> client_to_source_map_;
    std::mutex clients_mtx_;
    Discovery discovery_;
};

} // namespace uxr
} // namespace eprosima

#endif //_UXR_AGENT_TRANSPORT_UDP_SERVER_HPP_
