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

#ifndef _MICRORTPS_AGENT_PROCESSOR_PROCESSOR_HPP_
#define _MICRORTPS_AGENT_PROCESSOR_PROCESSOR_HPP_

#include <micrortps/agent/types/XRCETypes.hpp>
#include <micrortps/agent/types/MessageHeader.hpp>
#include <micrortps/agent/types/SubMessageHeader.hpp>
#include <micrortps/agent/message/Message.hpp>
#include <micrortps/agent/utils/Serializer.hpp>
#include <micrortps/agent/message/Packet.hpp>

namespace eprosima {
namespace micrortps {

class Agent;
class ProxyClient;

class Processor
{
public:
    Processor(Agent& root) : root_(root) {}

    void process_input_packet(InputPacket&& input_packet);

private:
    void process_input_message(ProxyClient& client, InputMessagePtr& input_message);
    bool process_submessage(ProxyClient& client, InputMessagePtr& input_message);
    bool process_create_client_submessage(InputMessagePtr& input_message);
    bool process_create_submessage(ProxyClient& client, InputMessagePtr& input_message);
    bool process_delete_submessage(ProxyClient& client, InputMessagePtr& input_message);
    bool process_write_data_submessage(ProxyClient& client, InputMessagePtr& input_message);
    bool process_read_data_submessage(ProxyClient& client, InputMessagePtr& input_message);
    bool process_acknack_submessage(ProxyClient& client, InputMessagePtr& input_message);
    bool process_heartbeat_submessage(ProxyClient& client, InputMessagePtr& input_message);

private:
    Agent& root_;
};

} // namespace micrortps
} // namespace eprosima

#endif //_MICRORTPS_AGENT_PROCESSOR_PROCESSOR_HPP_
