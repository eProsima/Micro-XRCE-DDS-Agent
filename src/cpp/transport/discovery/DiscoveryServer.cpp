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
#include <uxr/agent/transport/udp/UDPEndPoint.hpp>
#include <uxr/agent/processor/Processor.hpp>

#include <functional>

#define RECEIVE_TIMEOUT 100
#define DISCOVERY_IP "239.255.0.2"

namespace eprosima {
namespace uxr {

DiscoveryServer::DiscoveryServer(
        const Processor& processor,
        uint16_t agent_port)
    : running_cond_(false)
    , processor_(processor)
    , transport_address_{}
{
    dds::xrce::TransportAddressMedium transport_addr;
    transport_addr.port(agent_port);
    transport_address_.medium_locator(transport_addr);
}

bool DiscoveryServer::run(uint16_t discovery_port)
{
    if (running_cond_ || !init(discovery_port))
    {
        return false;
    }

    /* Init thread. */
    running_cond_ = true;
    discovery_thread_.reset(new std::thread(std::bind(&DiscoveryServer::discovery_loop, this)));

    return true;
}

bool DiscoveryServer::stop()
{
    /* Stop thread. */
    running_cond_ = false;
    if (discovery_thread_ && discovery_thread_->joinable())
    {
        discovery_thread_->join();
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
