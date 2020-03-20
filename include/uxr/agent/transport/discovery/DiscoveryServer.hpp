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

#ifndef UXR_AGENT_TRANSPORT_DISCOVERY_SERVER_HPP_
#define UXR_AGENT_TRANSPORT_DISCOVERY_SERVER_HPP_

#include <uxr/agent/message/Packet.hpp>
#include <uxr/agent/transport/endpoint/IPv4EndPoint.hpp>
#include <uxr/agent/transport/endpoint/IPv6EndPoint.hpp>

#include <thread>
#include <atomic>
#include <mutex>

namespace eprosima {
namespace uxr {

template<typename EndPoint>
class Processor;

template<typename EndPoint>
class DiscoveryServer
{
public:
    DiscoveryServer(
            const Processor<EndPoint>& processor);

    virtual ~DiscoveryServer() = default;

    template<typename T>
    bool run(
            uint16_t discovery_port,
            T&& transport_addresses);

    bool stop();

    void set_filter_port(uint16_t filter_port) { filter_port_ = filter_port; }

private:
    virtual bool init(
            uint16_t discovery_port) = 0;

    virtual bool close() = 0;

    virtual bool recv_message(
            InputPacket<IPv4EndPoint>& input_packet,
            int timeout) = 0;

    virtual bool send_message(
            OutputPacket<IPv4EndPoint>&& output_packet) = 0;

    void discovery_loop();

private:
    std::mutex mtx_;
    std::thread thread_;
    std::atomic<bool> running_cond_;
    const Processor<EndPoint>& processor_;

protected:
    std::vector<dds::xrce::TransportAddress> transport_addresses_;
    uint16_t agent_port_;
    uint16_t discovery_port_;
    uint16_t filter_port_;
};

template<typename EndPoint>
template<typename T>
inline
bool DiscoveryServer<EndPoint>::run(
        uint16_t discovery_port,
        T&& transport_addresses)
{
    std::lock_guard<std::mutex> lock(mtx_);

    transport_addresses_ = std::forward<T>(transport_addresses);
    if (running_cond_ || !init(discovery_port))
    {
        return false;
    }

    /* Init thread. */
    running_cond_ = true;
    thread_ = std::thread(&DiscoveryServer::discovery_loop, this);

    return true;
}

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_TRANSPORT_DISCOVERY_SERVER_HPP_
