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

Agent& root()
{
    static Agent xrce_agent;
    return xrce_agent;
}

Agent::Agent() :
    server_(),
    processor_(*this),
    response_thread_{},
    heartbeats_thread_(),
    reply_cond_(),
    heartbeat_cond_()
{
    running_ = false;
    reply_cond_ = false;
    heartbeat_cond_ = false;
}

bool Agent::init(XRCEServer* server)
{
    bool rv = false;
    if (nullptr != server)
    {
        server_ = server;
        rv = true;
    }
    return rv;
}

dds::xrce::ResultStatus Agent::create_client(const dds::xrce::CLIENT_Representation& client_representation,
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
                                                 std::forward_as_tuple(client_representation,
                                                                       client_key,
                                                                       session_id,
                                                                       addr,
                                                                       port)).second;
                if (create_result)
                {
#ifdef VERBOSE_OUTPUT
                    std::cout << "<== ";
                    debug::printl_connected_client_submessage(client_representation);
#endif
                    if (client_representation.session_id() > 127)
                    {
                        addr_to_key_.insert(std::make_pair(addr, client_key));
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
                                                     std::forward_as_tuple(client_representation,
                                                                           client_key,
                                                                           session_id,
                                                                           addr,
                                                                           port)).second;
                    if (create_result)
                    {
#ifdef VERBOSE_OUTPUT
                        debug::printl_connected_client_submessage(client_representation);
#endif
                        if (client_representation.session_id() > 127)
                        {
                            addr_to_key_.insert(std::make_pair(addr, client_key));
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

dds::xrce::ResultStatus Agent::delete_client(const dds::xrce::ClientKey& client_key)
{
    dds::xrce::ResultStatus result_status;
    ProxyClient* client = get_client(client_key);
    if (nullptr != client)
    {
        std::unique_lock<std::mutex> lock(clientsmtx_);
        addr_to_key_.erase(client->get_addr());
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

void Agent::run()
{
    std::cout << "Running DDS-XRCE Agent..." << std::endl;
    running_ = true;
//    uint32_t addr_udp = 0;
//    uint16_t port = 0;
    uint8_t* buf = nullptr;
    size_t len = 0;
    while(running_)
    {
        if (server_->recv_msg(&buf, &len, -1, &transport_client_))
        {
//            XrceMessage input_message = {reinterpret_cast<char*>(buf), len};
//            processor_.process_input_message(input_message, addr_udp, port);
            InputPacket input_packet;
            input_packet.client_key = {0xAA, 0xBB, 0xCC, 0xDD};
            input_packet.message.reset(new InputMessage(buf, len));
            processor_.process_input_packet(std::move(input_packet));
        }
    };
    std::cout << "Execution stopped" << std::endl;

}

void Agent::stop()
{
    running_ = false;
}

void Agent::abort_execution()
{
    reply_cond_ = false;
    heartbeat_cond_ = false;
    messages_.abort();
    if (response_thread_ && response_thread_->joinable())
    {
        response_thread_->join();
    }
    if (heartbeats_thread_ && heartbeats_thread_->joinable())
    {
        heartbeats_thread_->join();
    }
}

void Agent::add_reply(Message& message)
{
    messages_.push(message);
    if(response_thread_ == nullptr || !response_thread_->joinable())
    {
        reply_cond_ = true;
        response_thread_.reset(new std::thread(std::bind(&Agent::reply, this)));
    }
    if (heartbeats_thread_ == nullptr || !heartbeats_thread_->joinable())
    {
        heartbeat_cond_ = true;
        heartbeats_thread_.reset(new std::thread(std::bind(&Agent::manage_heartbeats, this)));
    }
}

void Agent::reply()
{
    while(reply_cond_)
    {
        Message message = messages_.pop();
        if (!messages_.is_aborted() && message.get_real_size() != 0)
        {
            server_->send_msg(reinterpret_cast<uint8_t*>(message.get_buffer().data()),
                              message.get_real_size(), transport_client_);
        }
    }
}

void Agent::manage_heartbeats()
{
    while (heartbeat_cond_)
    {
        /* Get clients. */
        for (auto it = clients_.begin(); it != clients_.end(); ++it)
        {
            ProxyClient& client = it->second;
            /* Get reliable streams. */
            for (auto s : client.session().get_output_streams())
            {
                /* Get and send  pending messages. */
                if (client.session().message_pending(s))
                {
                    /* Heartbeat message header. */
                    dds::xrce::MessageHeader heartbeat_header;
                    heartbeat_header.session_id(client.get_session_id());
                    heartbeat_header.stream_id(0x00);
                    heartbeat_header.sequence_nr(s);
                    heartbeat_header.client_key(client.get_client_key());

                    /* Heartbeat message payload. */
                    dds::xrce::HEARTBEAT_Payload heartbeat_payload;
                    heartbeat_payload.first_unacked_seq_nr(client.session().get_first_unacked_seq_nr(s));
                    heartbeat_payload.last_unacked_seq_nr(client.session().get_last_unacked_seq_nr(s));

                    /* Serialize heartbeat message. */
                    Message output_message{};
                    XRCEFactory message_creator{output_message.get_buffer().data(), output_message.get_buffer().max_size()};
                    message_creator.header(heartbeat_header);
                    message_creator.heartbeat(heartbeat_payload);
                    output_message.set_real_size(message_creator.get_total_size());
                    output_message.set_addr(client.get_addr());
                    output_message.set_port(client.get_port());

                    /* Send heartbeat. */
                    add_reply(output_message);
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(HEARTBEAT_PERIOD));
    }
}

ProxyClient* Agent::get_client(const dds::xrce::ClientKey& client_key)
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

ProxyClient* Agent::get_client(uint32_t addr)
{
    ProxyClient* client = nullptr;
    auto it = addr_to_key_.find(addr);
    if (it != addr_to_key_.end())
    {
        client = get_client(addr_to_key_.at(addr));
    }
    return client;
}

dds::xrce::ClientKey Agent::get_key(uint32_t addr)
{
    dds::xrce::ClientKey key = dds::xrce::CLIENTKEY_INVALID;
    auto it = addr_to_key_.find(addr);
    if (it != addr_to_key_.end())
    {
        key = addr_to_key_.at(addr);
    }
    return key;
}

} // namespace micrortps
} // namespace eprosima
