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
#include <uxr/agent/processor/Processor.hpp>

#include <functional>

#define RECEIVE_TIMEOUT 100

namespace eprosima {
namespace uxr {

extern template class Processor<IPv4EndPoint>;
extern template class Processor<IPv6EndPoint>;

template<typename EndPoint>
DiscoveryServer<EndPoint>::DiscoveryServer(
        const Processor<EndPoint>& processor)
    : mtx_{}
    , thread_{}
    , running_cond_{false}
    , processor_{processor}
    , transport_addresses_{}
    , agent_port_{}
    , discovery_port_{}
    , filter_port_{}
{}

template<typename EndPoint>
bool DiscoveryServer<EndPoint>::stop()
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

template<typename EndPoint>
void DiscoveryServer<EndPoint>::discovery_loop()
{
    InputPacket<IPv4EndPoint> input_packet;
    OutputPacket<IPv4EndPoint> output_packet;
    while (running_cond_)
    {
        if (recv_message(input_packet, RECEIVE_TIMEOUT))
        {
            if (processor_.process_get_info_packet(std::move(input_packet), transport_addresses_, output_packet))
            {
                send_message(std::move(output_packet));
            }
        }
    }
}

template class DiscoveryServer<IPv4EndPoint>;
template class DiscoveryServer<IPv6EndPoint>;

} // namespace uxr
} // namespace eprosima
