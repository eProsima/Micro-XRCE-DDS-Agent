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

Agent* root()
{
    static Agent xrce_agent;
    return &xrce_agent;
}

Agent::Agent() :
    loc_id_{},
    out_buffer_{},
    in_buffer_{},
    loc_{},
    response_thread_{},
    reply_cond_()
{
    running_ = false;
    reply_cond_ = false;
}

void Agent::init(const std::string& device)
{
    std::cout << "Serial agent initialization..." << std::endl;
    /* Init SERIAL transport */
    loc_id_ = add_serial_locator(device.data());
}

void Agent::init(uint16_t out_port, uint16_t in_port, uint16_t remote_port, const char* server_ip)
{
    std::cout << "UDP agent initialization..." << std::endl;
    // Init transport
    loc_id_ = add_udp_locator(out_port, in_port, remote_port, server_ip);
}

dds::xrce::ResultStatus Agent::create_client(const dds::xrce::CREATE_CLIENT_Payload& payload)
{
    dds::xrce::ResultStatus result_status;
    result_status.status(dds::xrce::STATUS_OK);

    if (payload.client_representation().xrce_cookie() == dds::xrce::XrceCookie{XRCE_COOKIE})
    {
        if (payload.client_representation().xrce_version()[0] == XRCE_VERSION_MAJOR)
        {
            std::lock_guard<std::mutex> lock(clientsmtx_);

            bool create_result;
            dds::xrce::ClientKey client_key = payload.client_representation().client_key();
            dds::xrce::SessionId session_id = payload.client_representation().session_id();
            auto it = clients_.find(client_key);
            if (it == clients_.end())
            {
                create_result = clients_.emplace(std::piecewise_construct,
                                                 std::forward_as_tuple(client_key),
                                                 std::forward_as_tuple(payload.client_representation(),
                                                                       client_key, session_id)).second;
                if (!create_result)
                {
                    result_status.status(dds::xrce::STATUS_ERR_RESOURCES);
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
                                                     std::forward_as_tuple(payload.client_representation(),
                                                                           client_key, session_id)).second;
                    if (!create_result)
                    {
                        result_status.status(dds::xrce::STATUS_ERR_RESOURCES);
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

dds::xrce::ResultStatus Agent::delete_client(dds::xrce::ClientKey client_key)
{
    dds::xrce::ResultStatus result_status;
    std::lock_guard<std::mutex> lock(clientsmtx_);
    if (0 ==clients_.erase(client_key))
    {
        result_status.status(dds::xrce::STATUS_ERR_INVALID_DATA);
    }
    else
    {
        result_status.status(dds::xrce::STATUS_OK);
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
        if (0 < (ret = receive_data(static_cast<octet*>(in_buffer_), buffer_len_, loc_id_)))
        {
//            XRCEParser myParser{reinterpret_cast<char*>(in_buffer_), static_cast<size_t>(ret), this};
//            myParser.parse();
            dds::xrce::XrceMessage input_message = {reinterpret_cast<char*>(in_buffer_), static_cast<size_t>(ret)};
            handle_input_message(input_message);
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
    messages_.abort();
    if (response_thread_ && response_thread_->joinable())
        response_thread_->join();
}

void Agent::add_reply(const Message& message)
{
    messages_.push(message);
    if(response_thread_ == nullptr)
    {
        reply_cond_ = true;
        response_thread_.reset(new std::thread(std::bind(&Agent::reply, this)));
    }
}


void Agent::add_reply(const dds::xrce::MessageHeader& header,
                      const dds::xrce::STATUS_Payload& status_reply)
{
#ifdef VERBOSE_OUTPUT
    std::cout << "<== ";
    eprosima::micrortps::debug::printl_status_submessage(status_reply);
#endif

    dds::xrce::MessageHeader updated_header{header};
    update_header(updated_header);
    Message message{};
    XRCEFactory message_creator{ message.get_buffer().data(), message.get_buffer().max_size() };
    message_creator.header(updated_header);
    message_creator.status(status_reply);
    message.set_real_size(message_creator.get_total_size());
    add_reply(message);
}

void Agent::add_reply(const dds::xrce::MessageHeader& header,
                      const dds::xrce::DATA_Payload_Data& payload)
{
#ifdef VERBOSE_OUTPUT
    std::cout << "<== ";
    eprosima::micrortps::debug::printl_data_submessage(payload);
#endif

    dds::xrce::MessageHeader updated_header{header};
    update_header(updated_header);
    Message message{};
    XRCEFactory message_creator{message.get_buffer().data(), message.get_buffer().max_size()};
    message_creator.header(updated_header);
    message_creator.data(payload);
    message.set_real_size(message_creator.get_total_size());
    add_reply(message);
}

void Agent::add_reply(const dds::xrce::MessageHeader& header,
                      const dds::xrce::DATA_Payload_Sample& payload)
{
/* TODO (Julian): implement verbose output functions. */
// #ifdef VERBOSE_OUTPUT
//     std::cout << "<== ";
//     eprosima::micrortps::debug::printl_data_submessage(payload);
// #endif

    dds::xrce::MessageHeader updated_header{header};
    update_header(updated_header);
    Message message{};
    XRCEFactory message_creator{message.get_buffer().data(), message.get_buffer().max_size()};
    message_creator.header(updated_header);
    message_creator.data(payload);
    message.set_real_size(message_creator.get_total_size());
    add_reply(message);
}

void Agent::add_reply(const dds::xrce::MessageHeader& header,
                      const dds::xrce::DATA_Payload_DataSeq& payload)
{
// #ifdef VERBOSE_OUTPUT
//     std::cout << "<== ";
//     eprosima::micrortps::debug::printl_data_submessage(payload);
// #endif

    dds::xrce::MessageHeader updated_header{header};
    update_header(updated_header);
    Message message{};
    XRCEFactory message_creator{message.get_buffer().data(), message.get_buffer().max_size()};
    message_creator.header(updated_header);
    message_creator.data(payload);
    message.set_real_size(message_creator.get_total_size());
    add_reply(message);
}

void Agent::add_reply(const dds::xrce::MessageHeader& header,
                      const dds::xrce::DATA_Payload_SampleSeq& payload)
{
// #ifdef VERBOSE_OUTPUT
//     std::cout << "<== ";
//     eprosima::micrortps::debug::printl_data_submessage(payload);
// #endif

    dds::xrce::MessageHeader updated_header{header};
    update_header(updated_header);
    Message message{};
    XRCEFactory message_creator{message.get_buffer().data(), message.get_buffer().max_size()};
    message_creator.header(updated_header);
    message_creator.data(payload);
    message.set_real_size(message_creator.get_total_size());
    add_reply(message);
}

void Agent::add_reply(const dds::xrce::MessageHeader& header,
                      const dds::xrce::DATA_Payload_PackedSamples& payload)
{
// #ifdef VERBOSE_OUTPUT
//     std::cout << "<== ";
//     eprosima::micrortps::debug::printl_data_submessage(payload);
// #endif

    dds::xrce::MessageHeader updated_header{header};
    update_header(updated_header);
    Message message{};
    XRCEFactory message_creator{message.get_buffer().data(), message.get_buffer().max_size()};
    message_creator.header(updated_header);
    message_creator.data(payload);
    message.set_real_size(message_creator.get_total_size());
    add_reply(message);
}

void Agent::add_reply(const dds::xrce::MessageHeader &header,
                      const dds::xrce::ACKNACK_Payload &payload)
{
    dds::xrce::MessageHeader updated_header{header};
    update_header(updated_header);
    Message message{};
    XRCEFactory message_creator{message.get_buffer().data(), message.get_buffer().max_size()};
    message_creator.header(updated_header);
    message_creator.acknack(payload);
    message.set_real_size(message_creator.get_total_size());
    add_reply(message);
}

void Agent::reply()
{
    while(reply_cond_)
    {
        Message message = messages_.pop();
        if (!message.get_buffer().empty())
        {
            int ret = 0;
            if (0 < (ret = send_data(reinterpret_cast<octet*>(message.get_buffer().data()), message.get_real_size(), loc_id_)))
            {
                printf("%d bytes response sent\n", ret);
            }
        }
    }
}

void Agent::on_message(const dds::xrce::MessageHeader& header,
                       const dds::xrce::SubmessageHeader& /*sub_header*/,
                       const dds::xrce::CREATE_CLIENT_Payload& create_client_payload)
{
#ifdef VERBOSE_OUTPUT
    std::cout << "==> ";
    eprosima::micrortps::debug::printl_create_client_submessage(create_client_payload);
#endif

    dds::xrce::STATUS_Payload status;
    status.related_request().request_id(create_client_payload.request_id());
    status.related_request().object_id(create_client_payload.object_id());
    dds::xrce::ResultStatus result_status = create_client(create_client_payload);
    status.result(result_status);
    add_reply(header, status);
}

void Agent::on_message(const dds::xrce::MessageHeader& header,
                       const dds::xrce::SubmessageHeader&  /*sub_header*/,
                       const dds::xrce::CREATE_Payload& create_payload)
{
#ifdef VERBOSE_OUTPUT
    std::cout << "==> ";
    eprosima::micrortps::debug::printl_create_submessage(create_payload);
#endif

    if (create_payload.object_representation()._d() != dds::xrce::OBJK_CLIENT)
    {
        if (ProxyClient* client = get_client(header.client_key()))
        {
            /* TODO (julian): check creation mode flags according to standard. */
            dds::xrce::CreationMode creation_mode;
            creation_mode.reuse(false);
            creation_mode.replace(true);

            // TODO(Borja): get sub_header flags
            // Bit 1, the ‘Reuse’ bit, encodes the value of the CreationMode reuse field.
            // Bit 2, the ‘Replace’ bit, encodes the value of the CreationMode replace field.
            dds::xrce::STATUS_Payload status;
            status.related_request().request_id(create_payload.request_id());
            status.related_request().object_id(create_payload.object_id());
            dds::xrce::ResultStatus result_status = client->create(creation_mode, create_payload);
            status.result(result_status);
            add_reply(header, status);
        }
        else
        {
            std::cerr << "Create message rejected" << std::endl;
        }
    }
    else if (create_payload.object_representation()._d() == dds::xrce::OBJK_CLIENT)
    {

    }
}

void Agent::on_message(const dds::xrce::MessageHeader& header,
                       const dds::xrce::SubmessageHeader&  /*sub_header*/,
                       const dds::xrce::DELETE_Payload& delete_payload)
{
#ifdef VERBOSE_OUTPUT
    std::cout << "==> ";
    eprosima::micrortps::debug::printl_delete_submessage(delete_payload);
#endif

    if (delete_payload.object_id() == dds::xrce::ObjectId{OBJECTID_CLIENT})
    {
        dds::xrce::STATUS_Payload status;
        status.related_request().request_id(delete_payload.request_id());
        status.related_request().object_id(delete_payload.object_id());
        status.result(delete_client(header.client_key()));
        add_reply(header, status);
    }
    else if (ProxyClient* client = get_client(header.client_key()))
    {
        dds::xrce::STATUS_Payload status;
        status.related_request().request_id(delete_payload.request_id());
        status.related_request().object_id(delete_payload.object_id());
        status.result(client->delete_object(delete_payload));
        add_reply(header, status);
    }
    else
    {
        std::cerr << "Delete message rejected" << std::endl;
    }
}

void Agent::on_message(const dds::xrce::MessageHeader& header,
                       const dds::xrce::SubmessageHeader&  /*sub_header*/,
                       dds::xrce::WRITE_DATA_Payload_Data& write_payload)
{
#ifdef VERBOSE_OUTPUT
    std::cout << "==> ";
    eprosima::micrortps::debug::printl_write_data_submessage(write_payload);
#endif

    if (ProxyClient* client = get_client(header.client_key()))
    {
        dds::xrce::STATUS_Payload status;
        status.related_request().request_id(write_payload.request_id());
        status.related_request().object_id(write_payload.object_id());
        dds::xrce::ResultStatus result_status = client->write(write_payload.object_id(), write_payload);
        status.result(result_status);
    }
    else
    {
        std::cerr << "Write message rejected" << std::endl;
    }
}

void Agent::on_message(const dds::xrce::MessageHeader &header,
                       const dds::xrce::SubmessageHeader& /*sub_header*/,
                       const dds::xrce::READ_DATA_Payload &read_payload)
{
#ifdef VERBOSE_OUTPUT
    std::cout << "==> ";
    eprosima::micrortps::debug::printl_read_data_submessage(read_payload);
#endif

    if(ProxyClient *client = get_client(header.client_key()))
    {
        dds::xrce::STATUS_Payload status;
        status.related_request().request_id(read_payload.request_id());
        status.related_request().object_id(read_payload.object_id());
        dds::xrce::ResultStatus result_status = client->read(read_payload.object_id(), read_payload);
        status.result(result_status);
    }
    else
    {
        std::cerr << "Read message rejected" << std::endl;
    }
}

void Agent::on_message(const dds::xrce::MessageHeader &header,
                       const dds::xrce::SubmessageHeader &sub_header,
                       const dds::xrce::ACKNACK_Payload &acknack_payload)
{
    (void) sub_header;
    (void) acknack_payload;
    if (ProxyClient *client = get_client(header.client_key()))
    {
        // TODO (julian): implement state machine for acknack messages.
        (void) client;
    }
}

void Agent::on_message(const dds::xrce::MessageHeader &header,
                       const dds::xrce::SubmessageHeader &sub_header,
                       const dds::xrce::HEARTBEAT_Payload &heartbeat_payload)
{
    (void) sub_header;
    (void) heartbeat_payload;
    if (ProxyClient *client = get_client(header.client_key()))
    {
        // TODO (julian): implement state machine for heartbeat message.
        dds::xrce::ACKNACK_Payload acknack;
        acknack.first_unacked_seq_num(heartbeat_payload.last_unacked_seq_nr());
        add_reply(header, acknack);
        (void) client;
    }
}

eprosima::micrortps::ProxyClient* Agent::get_client(dds::xrce::ClientKey client_key)
{
    try
    {
        std::lock_guard<std::mutex> lock(clientsmtx_);
        return &clients_.at(client_key);
    }
    catch (const std::out_of_range& e)
    {
        unsigned int key = client_key[0] + (client_key[1] << 8) + (client_key[2] << 16) + (client_key[3] << 24);
        std::cerr << "Client 0x" << std::hex << key << " not found" << std::endl;
        return nullptr;
    }
}

void Agent::update_header(dds::xrce::MessageHeader& header)
{
    // TODO(Borja): sequence number is general and not independent for each client
    static uint8_t sequence = 0;
    header.sequence_nr(sequence++);
}

void Agent::handle_input_message(const dds::xrce::XrceMessage& input_message)
{
    Serializer deserializer(input_message.buf, input_message.len);

    dds::xrce::MessageHeader header;
    if (deserializer.deserialize(header))
    {
        /* Create client message. */
        if ((header.stream_id() == dds::xrce::SESSIONID_NONE_WITHOUT_CLIENT_KEY)
                || (header.stream_id() == dds::xrce::SESSIONID_NONE_WITH_CLIENT_KEY))
        {
            dds::xrce::SubmessageHeader sub_header;
            deserializer.force_new_submessage_align();
            if (deserializer.deserialize(sub_header))
            {
                if (sub_header.submessage_id() == dds::xrce::CREATE_CLIENT)
                {
                    process_create_client(header, deserializer);
                }
            }
        }
        /* Process the rest of the messages. */
        else if (ProxyClient* client = get_client(header.client_key()))
        {
            StreamsManager& stream_manager = client->get_stream_manager();
            dds::xrce::StreamId stream_id = header.stream_id();
            uint16_t seq_num = header.sequence_nr();
            if (stream_manager.is_valid_message(stream_id, seq_num))
            {
                if (stream_manager.is_next_message(stream_id, seq_num))
                {
                    /* Process message. */
                    process_message(header, deserializer, *client);

                    /* Update sequence number. */
                    stream_manager.promote_seq_num(stream_id, seq_num);

                    /* Process next messages. */
                    while (stream_manager.message_available(stream_id))
                    {
                        /* Get and process next message. */
                        dds::xrce::XrceMessage next_message = stream_manager.get_next_message(stream_id);
                        Serializer temp_deserializer(next_message.buf, next_message.len);
                        process_message(header, temp_deserializer, *client);

                        /* Update sequence number. */
                        seq_num++;
                        stream_manager.promote_seq_num(stream_id, seq_num);
                    }
                }
                else
                {
                    /* Store message. */
                    stream_manager.store_message(stream_id, seq_num,
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
            case dds::xrce::DELETE:
                process_delete(header, sub_header, deserializer, client);
                break;
            case dds::xrce::WRITE_DATA:
                process_write_data(header, sub_header, deserializer, client);
                break;
            case dds::xrce::READ_DATA:
                process_read_data(header, sub_header, deserializer, client);
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

void Agent::process_create_client(const dds::xrce::MessageHeader& header, Serializer& deserializer)
{
    dds::xrce::CREATE_CLIENT_Payload payload;
    if (deserializer.deserialize(payload))
    {
        dds::xrce::STATUS_Payload status;
        status.related_request().request_id(payload.request_id());
        status.related_request().object_id(payload.object_id());
        dds::xrce::ResultStatus result = create_client(payload);
        status.result(result);
        add_reply(header, status);
    }
    else
    {
        std::cerr << "Error processing CREATE_CLIENT submessage." << std::endl;
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
        dds::xrce::STATUS_Payload status;
        status.related_request().request_id(payload.request_id());
        status.related_request().object_id(payload.object_id());
        dds::xrce::ResultStatus result =  client.create(creation_mode, payload);
        status.result(result);
        add_reply(header, status);
    }
    else
    {
        std::cerr << "Error processing CREATE submessage." << std::endl;
    }
}

void Agent::process_delete(const dds::xrce::MessageHeader& header,
                           const dds::xrce::SubmessageHeader& sub_header,
                           Serializer& deserializer, ProxyClient& client)
{
    dds::xrce::DELETE_Payload payload;
    if (deserializer.deserialize(payload))
    {
        if (payload.object_id() == dds::xrce::ObjectId{OBJECTID_CLIENT})
        {
            dds::xrce::STATUS_Payload status;
            status.related_request().request_id(payload.request_id());
            status.related_request().object_id(payload.object_id());
            status.result(delete_client(header.client_key()));
            add_reply(header, status);
        }
        else
        {
            dds::xrce::STATUS_Payload status;
            status.related_request().request_id(payload.request_id());
            status.related_request().object_id(payload.object_id());
            status.result(client.delete_object(payload));
            add_reply(header, status);
        }
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
    switch (flags)
    {
        case dds::xrce::FORMAT_DATA_F:
        {
            dds::xrce::WRITE_DATA_Payload_Data payload;
            if (deserializer.deserialize(payload))
            {
                dds::xrce::STATUS_Payload status;
                status.related_request().request_id(payload.request_id());
                status.related_request().object_id(payload.object_id());
                status.result(client.write(payload.object_id(), payload));
                status.result(result_status);
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
        dds::xrce::STATUS_Payload status;
        status.related_request().request_id(payload.request_id());
        status.related_request().object_id(payload.object_id());
        status.result(client.read(payload.object_id(), payload));
        add_reply(header, status);
    }
    else
    {
        std::cerr << "Error processing READ_DATA submessage." << std::endl;
    }
}

void Agent::process_acknack(const dds::xrce::MessageHeader& header,
                            const dds::xrce::SubmessageHeader& sub_header,
                            Serializer& deserializer, ProxyClient& client)
{
    dds::xrce::ACKNACK_Payload payload;
    if (deserializer.deserialize(payload))
    {
        /* Send again missing messages. */
    }
    else
    {
        std::cerr << "Error processing ACKNACK submessage." << std::endl;
    }
}

void Agent::process_heartbeat(const dds::xrce::MessageHeader& header,
                              const dds::xrce::SubmessageHeader& sub_header,
                              Serializer& deserializer, ProxyClient& client)
{
    dds::xrce::HEARTBEAT_Payload payload;
    if (deserializer.deserialize(payload))
    {
        client.get_stream_manager().update_from_heartbeat(header.stream_id(),
                                                          payload.first_unacked_seq_nr(),
                                                          payload.last_unacked_seq_nr());
    }
    else
    {
        std::cerr << "Error procession HEARTBEAT submessage." << std::endl;
    }
}

} // namespace micrortps
} // namespace eprosima
