// Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <agent/Root.h>

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
    locator_{},
    input_buffer_{},
    response_thread_{},
    heartbeats_thread_(),
    reply_cond_(),
    heartbeat_cond_()
{
    running_ = false;
    reply_cond_ = false;
    heartbeat_cond_ = false;
}

void Agent::init(const std::string& device)
{
    std::cout << "Serial agent initialization..." << std::endl;
    locator_id_t id = add_serial_locator(device.data(), &locator_);
    if (id != MICRORTPS_TRANSPORT_OK)
    {
        std::cout << "Agent::init() -> error" << std::endl;
    }
}

void Agent::init(const uint16_t local_port)
{
    std::cout << "UDP agent initialization..." << std::endl;
    locator_id_t id = add_udp_locator_agent(local_port, &locator_);
    if (id == MICRORTPS_TRANSPORT_ERROR)
    {
        std::cout << "Agent::init() -> error" << std::endl;
    }
}

dds::xrce::ResultStatus Agent::create_client(const dds::xrce::CLIENT_Representation& client_representation,
                                             dds::xrce::AGENT_Representation& /*agent_representation*/,
                                             uint32_t addr, uint16_t port)
{
    dds::xrce::ResultStatus result_status;
    result_status.status(dds::xrce::STATUS_OK);

    if (client_representation.xrce_cookie() == dds::xrce::XrceCookie{XRCE_COOKIE})
    {
        if (client_representation.xrce_version()[0] == XRCE_VERSION_MAJOR)
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
    int ret = 0;
    running_ = true;
    while(running_)
    {
        if (0 < (ret = receive_data(static_cast<uint8_t*>(input_buffer_), buffer_len_, locator_.locator_id)))
        {
            uint32_t addr = locator_.channel._.udp.remote_addr.sin_addr.s_addr;
            uint16_t port = locator_.channel._.udp.remote_addr.sin_port;
            XrceMessage input_message = {reinterpret_cast<char*>(input_buffer_), static_cast<size_t>(ret)};
            handle_input_message(input_message, addr, port);
        }
#ifdef WIN32
        Sleep(10);
#else
        usleep(10000);
#endif

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
    if(response_thread_ == nullptr)
    {
        reply_cond_ = true;
        response_thread_.reset(new std::thread(std::bind(&Agent::reply, this)));
    }
    if (heartbeats_thread_ == nullptr)
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
            int ret = 0;
            locator_.channel._.udp.remote_addr.sin_addr.s_addr = message.get_addr();
            locator_.channel._.udp.remote_addr.sin_port = message.get_port();
            if (0 < (ret = send_data(reinterpret_cast<uint8_t*>(message.get_buffer().data()), message.get_real_size(),
                     locator_.locator_id)))
            {
//                printf("%d bytes response sent\n", ret);
            }
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
            for (auto s : client.stream_manager().get_output_streams())
            {
                /* Get and send  pending messages. */
                if (client.stream_manager().message_pending(s))
                {
                    /* Heartbeat message header. */
                    dds::xrce::MessageHeader heartbeat_header;
                    heartbeat_header.session_id(client.get_session_id());
                    heartbeat_header.stream_id(0x00);
                    heartbeat_header.sequence_nr(s);
                    heartbeat_header.client_key(client.get_client_key());

                    /* Heartbeat message payload. */
                    dds::xrce::HEARTBEAT_Payload heartbeat_payload;
                    heartbeat_payload.first_unacked_seq_nr(client.stream_manager().get_first_unacked_seq_nr(s));
                    heartbeat_payload.last_unacked_seq_nr(client.stream_manager().get_last_unacked_seq_nr(s));

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
    dds::xrce::ClientKey key = CLIENTKEY_INVALID;
    auto it = addr_to_key_.find(addr);
    if (it != addr_to_key_.end())
    {
        key = addr_to_key_.at(addr);
    }
    return key;
}

void Agent::handle_input_message(const XrceMessage& input_message, uint32_t addr, uint16_t port)
{
    Serializer deserializer(input_message.buf, input_message.len);

    dds::xrce::MessageHeader header;
    if (deserializer.deserialize(header))
    {
        /* Create client message. */
        if ((header.session_id() == dds::xrce::SESSIONID_NONE_WITHOUT_CLIENT_KEY)
                || (header.session_id() == dds::xrce::SESSIONID_NONE_WITH_CLIENT_KEY))
        {
            dds::xrce::SubmessageHeader sub_header;
            deserializer.force_new_submessage_align();
            if (deserializer.deserialize(sub_header))
            {
                if (sub_header.submessage_id() == dds::xrce::CREATE_CLIENT)
                {
                    process_create_client(header, deserializer, addr, port);
                }
                if (sub_header.submessage_id() == dds::xrce::DELETE_ID)
                {
                    process_delete_client(header, deserializer, addr, port);
                }
            }
        }
        /* Process the rest of the messages. */
        else
        {
            ProxyClient* client = nullptr;
            if (header.session_id() < 128)
            {
                client = get_client(header.client_key());
            }
            else
            {
                client = get_client(addr);
            }

            if (nullptr != client)
            {
                StreamsManager& stream_manager = client->stream_manager();
                dds::xrce::StreamId stream_id = header.stream_id();
                uint16_t seq_num = header.sequence_nr();
                if (stream_manager.is_valid_message(stream_id, seq_num))
                {
                    if (stream_manager.is_next_message(stream_id, seq_num))
                    {
                        /* Process message. */
                        process_message(header, deserializer, *client);

                        /* Promote sequence number. */
                        stream_manager.promote_stream(stream_id, seq_num);

                        /* Process next messages. */
                        while (stream_manager.message_available(stream_id))
                        {
                            /* Get and process next messages. */
                            XrceMessage next_message = stream_manager.get_next_message(stream_id);
                            Serializer temp_deserializer(next_message.buf, next_message.len);
                            process_message(header, temp_deserializer, *client);

                            /* Update stream. */
                            stream_manager.promote_stream(stream_id, ++seq_num);
                        }
                    }
                    else
                    {
                        /* Store message. */
                        stream_manager.store_input_message(stream_id, seq_num,
                                                           deserializer.get_current_position(),
                                                           deserializer.get_remainder_size());
                    }
                }
            }
            else
            {
                std::cerr << "Error client unknown." << std::endl;
            }
        }
    }
    else
    {
        std::cerr << "Error reading message header." << std::endl;
    }
}

void Agent::process_message(const dds::xrce::MessageHeader& header, Serializer& deserializer, ProxyClient& client)
{

    dds::xrce::SubmessageHeader sub_header;
    deserializer.force_new_submessage_align();
    bool valid_submessage = deserializer.deserialize(sub_header);
    if (!valid_submessage)
    {
        std::cerr << "Error reading submessage header." << std::endl;
    }

    /* Process submessages. */
    while (valid_submessage)
    {
        switch (sub_header.submessage_id())
        {
            case dds::xrce::CREATE:
                process_create(header, sub_header, deserializer, client);
                break;
            case dds::xrce::GET_INFO:
                /* TODO (Julian). */
                break;
            case dds::xrce::DELETE_ID:
                process_delete(header, sub_header, deserializer, client);
                break;
            case dds::xrce::WRITE_DATA:
                process_write_data(header, sub_header, deserializer, client);
                break;
            case dds::xrce::READ_DATA:
                process_read_data(header, sub_header, deserializer, client);
                break;
            case dds::xrce::HEARTBEAT:
                process_heartbeat(header, sub_header, deserializer, client);
                break;
            case dds::xrce::ACKNACK:
                process_acknack(header, sub_header, deserializer, client);
                break;
            default:
                break;
        }

        /* Get next submessage. */
        if (!deserializer.bufferEnd())
        {
            deserializer.force_new_submessage_align();
            valid_submessage = deserializer.deserialize(sub_header);
            if (!valid_submessage)
            {
                std::cerr << "Error reading submessage header." << std::endl;
            }
        }
        else
        {
            valid_submessage = false;
        }
    }
}

void Agent::process_create_client(const dds::xrce::MessageHeader& header,
                                  Serializer& deserializer,
                                  uint32_t addr, uint16_t port)
{
    dds::xrce::CREATE_CLIENT_Payload client_payload;
    if (deserializer.deserialize(client_payload))
    {
        dds::xrce::STATUS_Payload status_payload;
        status_payload.related_request().request_id(client_payload.request_id());
        status_payload.related_request().object_id(client_payload.object_id());
        dds::xrce::AGENT_Representation agent_representation;
        status_payload.result(create_client(client_payload.client_representation(), agent_representation, addr, port));

        /* Send status message. */
        Message status_message{};
        XRCEFactory message_creator{status_message.get_buffer().data(), status_message.get_buffer().max_size()};
        message_creator.header(header);
        message_creator.status(status_payload);
        status_message.set_real_size(message_creator.get_total_size());
        status_message.set_addr(addr);
        status_message.set_port(port);
        add_reply(status_message);
    }
    else
    {
        std::cerr << "Error processing CREATE_CLIENT submessage." << std::endl;
    }
}

void Agent::process_delete_client(const dds::xrce::MessageHeader &header,
                                  Serializer &deserializer,
                                  uint32_t addr, uint16_t port)
{
    dds::xrce::DELETE_Payload delete_payload;
    if (deserializer.deserialize(delete_payload))
    {
        dds::xrce::STATUS_Payload status_payload;
        status_payload.related_request().request_id(delete_payload.request_id());
        status_payload.related_request().object_id(delete_payload.object_id());
        dds::xrce::ClientKey client_key;
        if (header.session_id() < 128)
        {
            client_key = header.client_key();
        }
        else
        {
            client_key = get_key(addr);
        }
        status_payload.result(delete_client(client_key));

        Message status_message{};
        XRCEFactory message_creator{status_message.get_buffer().data(), status_message.get_buffer().max_size()};
        message_creator.header(header);
        message_creator.status(status_payload);
        status_message.set_real_size(message_creator.get_total_size());
        status_message.set_addr(addr);
        status_message.set_port(port);
        add_reply(status_message);
    }
    else
    {
        std::cerr << "Error processing DELETE submessage." << std::endl;
    }
}

void Agent::process_create(const dds::xrce::MessageHeader& header,
                           const dds::xrce::SubmessageHeader& sub_header,
                           Serializer& deserializer,
                           ProxyClient& client)
{
    dds::xrce::CreationMode creation_mode;
    bool reuse = ((sub_header.flags() & dds::xrce::FLAG_REUSE) == dds::xrce::FLAG_REUSE);
    bool replace = ((sub_header.flags() & dds::xrce::FLAG_REPLACE) == dds::xrce::FLAG_REPLACE);
    creation_mode.reuse(reuse);
    creation_mode.replace(replace);

    dds::xrce::CREATE_Payload payload;
    if (deserializer.deserialize(payload))
    {
#ifdef VERBOSE_OUTPUT
        std::cout << "<== ";
        debug::printl_create_submessage(client.get_client_key(), payload);
#endif
        /* Status message header. */
        dds::xrce::MessageHeader status_header;
        status_header.session_id(header.session_id());
        uint8_t stream_id = 0x80;
        status_header.stream_id(stream_id);
        uint16_t seq_num = client.stream_manager().next_ouput_message(stream_id);
        status_header.sequence_nr(seq_num);
        status_header.client_key(header.client_key());

        /* Status payload. */
        dds::xrce::STATUS_Payload status_payload;
        status_payload.related_request().request_id(payload.request_id());
        status_payload.related_request().object_id(payload.object_id());
        status_payload.result(client.create(creation_mode, payload.object_id(), payload.object_representation()));

        /* Serialize status. */
        Message message{};
        XRCEFactory message_creator{message.get_buffer().data(), message.get_buffer().max_size()};
        message_creator.header(status_header);
        message_creator.status(status_payload);
        message.set_real_size(message_creator.get_total_size());
        message.set_addr(client.get_addr());
        message.set_port(client.get_port());

        /* Store message. */
        client.stream_manager().store_output_message(stream_id, message.get_buffer().data(), message.get_real_size());

        /* Send status. */
        add_reply(message);
    }
    else
    {
        std::cerr << "Error processing CREATE submessage." << std::endl;
    }
}

void Agent::process_delete(const dds::xrce::MessageHeader& header,
                           const dds::xrce::SubmessageHeader& /*sub_header*/,
                           Serializer& deserializer, ProxyClient& client)
{
    dds::xrce::DELETE_Payload payload;
    if (deserializer.deserialize(payload))
    {
        /* Status message header. */
        dds::xrce::MessageHeader status_header;
        status_header.session_id(header.session_id());
        uint8_t stream_id = 0x80;
        status_header.stream_id(stream_id);
        uint16_t seq_num = client.stream_manager().next_ouput_message(stream_id);
        status_header.sequence_nr(seq_num);
        status_header.client_key(header.client_key());

        /* Status payload. */
        dds::xrce::STATUS_Payload status_payload;
        status_payload.related_request().request_id(payload.request_id());
        status_payload.related_request().object_id(payload.object_id());
        status_payload.result(client.delete_object(payload.object_id()));

        /* Serialize status. */
        Message message{};
        XRCEFactory message_creator{message.get_buffer().data(), message.get_buffer().max_size()};
        message_creator.header(status_header);
        message_creator.status(status_payload);
        message.set_real_size(message_creator.get_total_size());
        message.set_addr(client.get_addr());
        message.set_port(client.get_port());

        /* Store message. */
        client.stream_manager().store_output_message(stream_id, message.get_buffer().data(), message.get_real_size());

        /* Send status. */
        add_reply(message);
    }
    else
    {
        std::cerr << "Error processing DELETE submessage." << std::endl;
    }
}

void Agent::process_write_data(const dds::xrce::MessageHeader& /*header*/,
                               const dds::xrce::SubmessageHeader& sub_header,
                               Serializer& deserializer, ProxyClient& client)
{
    uint8_t flags = sub_header.flags() & 0x0E;
    dds::xrce::DataRepresentation data;
    switch (flags)
    {
        case dds::xrce::FORMAT_DATA_F: ;
        {
            dds::xrce::WRITE_DATA_Payload_Data payload;
            if (deserializer.deserialize(payload))
            {
                DataWriter* data_writer = dynamic_cast<DataWriter*>(client.get_object(payload.object_id()));
                if (nullptr != data_writer)
                {
                    data_writer->write(payload);
                }
            }
            break;
        }
        default:
            break;
    }
}

void Agent::process_read_data(const dds::xrce::MessageHeader& header,
                              const dds::xrce::SubmessageHeader& /*sub_header*/,
                              Serializer& deserializer, ProxyClient& client)
{
    dds::xrce::READ_DATA_Payload payload;
    if (deserializer.deserialize(payload))
    {
        /* Status message header. */
        dds::xrce::MessageHeader status_header;
        status_header.session_id(header.session_id());
        uint8_t stream_id = 0x80;
        status_header.stream_id(stream_id);
        uint16_t seq_num = client.stream_manager().next_ouput_message(stream_id);
        status_header.sequence_nr(seq_num);
        status_header.client_key(header.client_key());

        /* Status payload. */
        dds::xrce::STATUS_Payload status_payload;
        status_payload.related_request().request_id(payload.request_id());
        status_payload.related_request().object_id(payload.object_id());
        dds::xrce::ResultStatus result;
        result.implementation_status(0x00);
        DataReader* data_reader = dynamic_cast<DataReader*>(client.get_object(payload.object_id()));
        if (nullptr != data_reader)
        {
            data_reader->read(payload, header.stream_id());
            result.status(dds::xrce::STATUS_OK);
        }
        else
        {
            result.status(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
        }
        status_payload.result(result);

        /* Serialize status. */
        Message message{};
        XRCEFactory message_creator{message.get_buffer().data(), message.get_buffer().max_size()};
        message_creator.header(status_header);
        message_creator.status(status_payload);
        message.set_real_size(message_creator.get_total_size());
        message.set_addr(client.get_addr());
        message.set_port(client.get_port());

        /* Store message. */
        client.stream_manager().store_output_message(stream_id, message.get_buffer().data(), message.get_real_size());

        /* Send status. */
        add_reply(message);
    }
    else
    {
        std::cerr << "Error processing READ_DATA submessage." << std::endl;
    }
}

void Agent::process_acknack(const dds::xrce::MessageHeader& header,
                            const dds::xrce::SubmessageHeader& /*sub_header*/,
                            Serializer& deserializer, ProxyClient& client)
{
    dds::xrce::ACKNACK_Payload payload;
    if (deserializer.deserialize(payload))
    {
        /* Send missing messages again. */
        uint16_t first_message = payload.first_unacked_seq_num();
        std::array<uint8_t, 2> nack_bitmap = payload.nack_bitmap();
        for (uint16_t i = 0; i < 8; ++i)
        {
            XrceMessage message;
            uint8_t mask = 0x01 << i;
            if ((nack_bitmap.at(1) & mask) == mask)
            {
                message = client.stream_manager().get_output_message((uint8_t) header.sequence_nr(), add_seq_num(first_message, i));
                if (message.len != 0)
                {
                    Message output_message(message.buf, message.len);
                    output_message.set_addr(client.get_addr());
                    output_message.set_port(client.get_port());
                    add_reply(output_message);
                }
            }
            if ((nack_bitmap.at(0) & mask) == mask)
            {
                message = client.stream_manager().get_output_message((uint8_t) header.sequence_nr(), add_seq_num(first_message, i + 8));
                if (message.len != 0)
                {
                    Message output_message(message.buf, message.len);
                    output_message.set_addr(client.get_addr());
                    output_message.set_port(client.get_port());
                    add_reply(output_message);
                }
            }
        }

        /* Update output stream. */
        client.stream_manager().update_from_acknack((uint8_t) header.sequence_nr(), first_message);
    }
    else
    {
        std::cerr << "Error processing ACKNACK submessage." << std::endl;
    }
}

void Agent::process_heartbeat(const dds::xrce::MessageHeader& header,
                              const dds::xrce::SubmessageHeader& /*sub_header*/,
                              Serializer& deserializer, ProxyClient& client)
{
    dds::xrce::HEARTBEAT_Payload payload;
    if (deserializer.deserialize(payload))
    {
        /* Update input stream. */
        dds::xrce::StreamId stream_id = static_cast<dds::xrce::StreamId>(header.sequence_nr());
        client.stream_manager().update_from_heartbeat(stream_id,
                                                      payload.first_unacked_seq_nr(),
                                                      payload.last_unacked_seq_nr());

        /* Send ACKNACK message. */
        dds::xrce::MessageHeader acknack_header;
        acknack_header.session_id() = header.session_id();
        acknack_header.stream_id() = 0x00;
        acknack_header.sequence_nr() = header.sequence_nr();
        acknack_header.client_key() = header.client_key();

        dds::xrce::ACKNACK_Payload acknack_payload;
        acknack_payload.first_unacked_seq_num(client.stream_manager().get_first_unacked_seq_num(stream_id));
        acknack_payload.nack_bitmap(client.stream_manager().get_nack_bitmap(stream_id));

        Message message{};
        XRCEFactory message_creator{message.get_buffer().data(), message.get_buffer().max_size()};
        message_creator.header(acknack_header);
        message_creator.acknack(acknack_payload);
        message.set_real_size(message_creator.get_total_size());
        message.set_addr(client.get_addr());
        message.set_port(client.get_port());
        add_reply(message);
    }
    else
    {
        std::cerr << "Error procession HEARTBEAT submessage." << std::endl;
    }
}

} // namespace micrortps
} // namespace eprosima
