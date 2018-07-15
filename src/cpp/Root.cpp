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

#include <micrortps/agent/Root.hpp>
#include <libdev/MessageDebugger.h>
#include <libdev/MessageOutput.h>
#include <fastcdr/Cdr.h>
#include <memory>

#ifdef WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

namespace eprosima {
namespace micrortps {

Root::Root()
    : clientsmtx_(),
      clients_()
{
}

dds::xrce::ResultStatus Root::create_client(const dds::xrce::CLIENT_Representation& client_representation,
                                             dds::xrce::AGENT_Representation& agent_representation,
                                             uint32_t addr, uint16_t port)
{
    dds::xrce::ResultStatus result_status;
    result_status.status(dds::xrce::STATUS_OK);

    if (client_representation.xrce_cookie() == dds::xrce::XRCE_COOKIE)
    {
        if (client_representation.xrce_version()[0] == dds::xrce::XRCE_VERSION_MAJOR)
        {
            std::lock_guard<std::mutex> lock(clientsmtx_);

            bool create_result;
            dds::xrce::ClientKey client_key = client_representation.client_key();
            dds::xrce::SessionId session_id = client_representation.session_id();
            auto it = clients_.find(client_key);
            if (it == clients_.end())
            {
                create_result = clients_.emplace(std::piecewise_construct,
                                                 std::forward_as_tuple(client_key),
                                                 std::forward_as_tuple(client_representation)).second;
                if (create_result)
                {
#ifdef VERBOSE_OUTPUT
                    std::cout << "<== ";
                    debug::printl_connected_client_submessage(client_representation);
#endif
                    if (client_representation.session_id() > 127)
                    {
//                        addr_to_key_.insert(std::make_pair(addr, client_key));
                    }
                }
                else
                {
                    result_status.status(dds::xrce::STATUS_ERR_DDS_ERROR);
                }
            }
            else
            {
                ProxyClient& client = clients_.at(client_key);
                if (session_id != client.get_session_id())
                {
                    clients_.erase(it);
                    create_result = clients_.emplace(std::piecewise_construct,
                                                     std::forward_as_tuple(client_key),
                                                     std::forward_as_tuple(client_representation)).second;
                    if (create_result)
                    {
#ifdef VERBOSE_OUTPUT
                        debug::printl_connected_client_submessage(client_representation);
#endif
                        if (client_representation.session_id() > 127)
                        {
//                            addr_to_key_.insert(std::make_pair(addr, client_key));
                        }
                    }
                    else
                    {
                        result_status.status(dds::xrce::STATUS_ERR_DDS_ERROR);
                    }
                }
            }
        }
        else
        {
            result_status.status(dds::xrce::STATUS_ERR_INCOMPATIBLE);
        }
    }
    else
    {
        result_status.status(dds::xrce::STATUS_ERR_INVALID_DATA);
    }

    // TODO (julian): measure time.
    dds::xrce::Time_t timestamp;
    timestamp.seconds(0);
    timestamp.nanoseconds(0);
    agent_representation.agent_timestamp(timestamp);
    agent_representation.xrce_cookie(dds::xrce::XRCE_COOKIE);
    agent_representation.xrce_version(dds::xrce::XRCE_VERSION);
    agent_representation.xrce_vendor_id(dds::xrce::XrceVendorId{EPROSIMA_VENDOR_ID});

    return result_status;
}

dds::xrce::ResultStatus Root::delete_client(const dds::xrce::ClientKey& client_key)
{
    dds::xrce::ResultStatus result_status;
    ProxyClient* client = get_client(client_key);
    if (nullptr != client)
    {
        std::unique_lock<std::mutex> lock(clientsmtx_);
//        addr_to_key_.erase(client->get_addr());
        clients_.erase(client_key);
        lock.unlock();
        result_status.status(dds::xrce::STATUS_OK);
    }
    else
    {
        result_status.status(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
    }
    return result_status;
}

//void Root::manage_heartbeats()
//{
//    while (heartbeat_cond_)
//    {
//        /* Get clients. */
//        for (auto it = clients_.begin(); it != clients_.end(); ++it)
//        {
//            ProxyClient& client = it->second;
//            /* Get reliable streams. */
//            for (auto s : client.session().get_output_streams())
//            {
//                /* Get and send  pending messages. */
//                if (client.session().message_pending(s))
//                {
//                    /* Heartbeat message header. */
//                    dds::xrce::MessageHeader heartbeat_header;
//                    heartbeat_header.session_id(client.get_session_id());
//                    heartbeat_header.stream_id(0x00);
//                    heartbeat_header.sequence_nr(s);
//                    heartbeat_header.client_key(client.get_client_key());
//
//                    /* Heartbeat message payload. */
//                    dds::xrce::HEARTBEAT_Payload heartbeat_payload;
//                    heartbeat_payload.first_unacked_seq_nr(client.session().get_first_unacked_seq_nr(s));
//                    heartbeat_payload.last_unacked_seq_nr(client.session().get_last_unacked_seq_nr(s));
//
//                    /* Serialize heartbeat message. */
//                    OutputMessagePtr output_message(new OutputMessage(heartbeat_header));
//                    output_message->append_submessage(dds::xrce::HEARTBEAT, heartbeat_payload);
//
//                    /* Send heartbeat. */
//                    add_reply(output_message);
//                }
//            }
//        }
//        std::this_thread::sleep_for(std::chrono::milliseconds(HEARTBEAT_PERIOD));
//    }
//}

ProxyClient* Root::get_client(const dds::xrce::ClientKey& client_key)
{
    ProxyClient* client = nullptr;
    std::lock_guard<std::mutex> lock(clientsmtx_);
    auto it = clients_.find(client_key);
    if (it != clients_.end())
    {
        client = &clients_.at(client_key);
    }
    return client;
}

} // namespace micrortps
} // namespace eprosima
