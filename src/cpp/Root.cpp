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

dds::xrce::ResultStatus Agent::create_client(const dds::xrce::MessageHeader& header,
                                             const dds::xrce::CREATE_CLIENT_Payload& create_info)
{
    dds::xrce::ResultStatus result_status;

    if (create_info.client_representation().xrce_cookie() == dds::xrce::XrceCookie{XRCE_COOKIE})
    {
        if (create_info.client_representation().xrce_version()[0] == XRCE_VERSION_MAJOR)
        {
            // TODO(Borja): The Agent shall check the ClientKey to ensure it is authorized to connect to the Agent
            // If this check fails the operation shall fail and returnValue is set to {STATUS_LAST_OP_CREATE,STATUS_ERR_DENIED}.

            // TODO(Borja): Check if there is an existing DdsXrce::ProxyClient object associated with the same ClientKey and if
            // so compare the session_id of the existing ProxyClient with the one in the object_representation:
            // o If a ProxyClient exists and has the same session_id then the operation shall not perform any action
            // and shall set the returnValue to {STATUS_LAST_OP_CREATE,STATUS_OK}.
            // o If a ProxyClient exist and has a different session_id then the operation shall delete the existing
            // DdsXrce::ProxyClient object and shall proceed as if the ProxyClient did not exist.
            // Check there are sufficient internal resources to complete the create operation. If there are not, then the operation
            // shall fail and set the returnValue to {STATUS_LAST_OP_CREATE, STATUS_ERR_RESOURCES}.
            std::lock_guard<std::mutex> lock(clientsmtx_);
            // auto client_it      = clients_.find(header.client_key());
            // if (client_it != clients_.end())
            // {
            //     clients_.erase(header.client_key());
            // }
            clients_[header.client_key()] = ProxyClient{create_info.client_representation(), header};
            //std::cout << "ProxyClient created " << std::endl;
            result_status.status(dds::xrce::STATUS_OK);
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
            XRCEParser myParser{reinterpret_cast<char*>(in_buffer_), static_cast<size_t>(ret), this};
            myParser.parse();
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
    dds::xrce::ResultStatus result_status = create_client(header, create_client_payload);
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

} // namespace micrortps
} // namespace eprosima
