// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <uxr/agent/transport/p2p/AgentDiscoverer.hpp>
#include <uxr/agent/p2p/InternalClientManager.hpp>

namespace eprosima {
namespace uxr {

AgentDiscoverer::AgentDiscoverer(
        Agent& agent)
    : agent_(agent)
    , mtx_{}
    , thread_{}
    , running_cond_{false}
{}

bool AgentDiscoverer::start(
        uint16_t p2p_port,
        uint16_t agent_port)
{
    std::lock_guard<std::mutex> lock(mtx_);

    if (running_cond_ || !init(p2p_port))
    {
        return false;
    }

    InternalClientManager& manager = InternalClientManager::instance();
    manager.set_local_address(agent_port);
    thread_ = std::thread(&AgentDiscoverer::loop, this);
    running_cond_ = true;
    return true;
}

bool AgentDiscoverer::stop()
{
    std::lock_guard<std::mutex> lock(mtx_);

    /* Stop thread. */
    running_cond_ = false;
    if (thread_.joinable())
    {
        thread_.join();
    }

    InternalClientManager& manager = InternalClientManager::instance();
    manager.delete_clients();

    return fini();
}

void AgentDiscoverer::loop()
{
    /* Header. */
    dds::xrce::MessageHeader header;
    header.session_id(dds::xrce::SESSIONID_NONE_WITHOUT_CLIENT_KEY);
    header.stream_id(dds::xrce::STREAMID_NONE);
    header.sequence_nr(0x0000);

    /* GET_INFO subheader. */
    dds::xrce::SubmessageHeader subheader;
    subheader.submessage_id(dds::xrce::GET_INFO);
    subheader.flags(dds::xrce::FLAG_LITTLE_ENDIANNESS);

    /* GET_INFO payload. */
    dds::xrce::GET_INFO_Payload payload;
    payload.info_mask(dds::xrce::INFO_ACTIVITY);

    /* Compute message size. */
    const size_t message_size =
            header.getCdrSerializedSize() +
            subheader.getCdrSerializedSize() +
            payload.getCdrSerializedSize();

    OutputMessage output_message{header, message_size};
    output_message.append_submessage(dds::xrce::GET_INFO, payload);
    InputMessagePtr input_message;

    while (running_cond_)
    {
        send_message(output_message);
        bool message_received;
        do
        {
            message_received = recv_message(input_message, 0);
            if (message_received)
            {
                dds::xrce::INFO_Payload info_payload;
                input_message->prepare_next_submessage();
                input_message->get_payload(info_payload);
                dds::xrce::TransportAddressMedium address =
                        info_payload.object_info().activity().agent().address_seq()[0].medium_locator();
                InternalClientManager& manager = InternalClientManager::instance();
                manager.create_client(agent_, address.address(), address.port());
            }
        } while(message_received);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

} // namespace uxr
} // namespace eprosima
