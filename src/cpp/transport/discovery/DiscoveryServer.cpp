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

#include <uxr/agent/transport/discovery/DiscoveryServer.hpp>
#include <uxr/agent/transport/endpoint/IPv4EndPoint.hpp>
#include <uxr/agent/processor/Processor.hpp>

#include <functional>

#define RECEIVE_TIMEOUT 100

namespace eprosima {
namespace uxr {

DiscoveryServer::DiscoveryServer(const Processor& processor)
    : mtx_{}
    , thread_{}
    , running_cond_{false}
    , processor_{processor}
    , transport_address_{}
    , filter_port_{}
{
}

bool DiscoveryServer::run(
        uint16_t discovery_port,
        const dds::xrce::TransportAddress& local_address)
{
    std::lock_guard<std::mutex> lock(mtx_);

    if (running_cond_ || !init(discovery_port))
    {
        return false;
    }

    /* Set transport address. */
    transport_address_ = local_address;

    /* Init thread. */
    running_cond_ = true;
    thread_ = std::thread(&DiscoveryServer::discovery_loop, this);

    return true;
}

bool DiscoveryServer::stop()
{
    std::lock_guard<std::mutex> lock(mtx_);

    /* Stop thread. */
    running_cond_ = false;
    if (thread_.joinable())
    {
        thread_.join();
    }
    return close();
}

void DiscoveryServer::discovery_loop()
{
    InputPacket input_packet;
    OutputPacket output_packet;
    while (running_cond_)
    {
        if (recv_message(input_packet, RECEIVE_TIMEOUT))
        {
            if (processor_.process_get_info_packet(std::move(input_packet), transport_address_, output_packet))
            {
                send_message(std::move(output_packet));
            }
        }
    }
}

} // namespace uxr
} // namespace eprosima
