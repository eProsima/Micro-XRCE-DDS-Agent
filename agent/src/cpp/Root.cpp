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

#include "agent/Root.h"

#include "agent/Payloads.h"
#include "agent/MessageHeader.h"
#include "agent/SubMessageHeader.h"

using namespace eprosima::micrortps;

Agent* eprosima::micrortps::root()
{
    static Agent xrce_agent;
    return &xrce_agent;
}

Agent::Agent() :
    ch_id_{},
    out_buffer_{},
    in_buffer_{},
    loc_{},
    clients_{},
    response_thread_{},
    response_control_{},
    messages_{}
{
    response_control_.running_ = false;
    response_control_.run_scheduled_ = false;
}

void Agent::init()
{
    std::cout << "Agent initialization" << std::endl;
    // // Init transport
    // loc = locator_t{LOC_SERIAL, "/dev/ttyACM0"};
    // ch_id = add_locator(&loc);
}

Status Agent::create_client(int32_t client_key, const CREATE_PAYLOAD& create_info)
{
    Status status;
    status.result().request_id(create_info.request_id());
    status.result().status(STATUS_LAST_OP_CREATE);
    status.object_id(create_info.object_id());

    if ((create_info.object_representation().discriminator() == OBJK_CLIENT) && 
        (create_info.object_representation().client().xrce_cookie() == std::array<uint8_t, 4>XRCE_COOKIE))
    {
        if (create_info.object_representation().client().xrce_version()[0] <= XRCE_VERSION_MAJOR)
        {
            // TODO The Agent shall check the ClientKey to ensure it is authorized to connect to the Agent
            // If this check fails the operation shall fail and returnValue is set to {STATUS_LAST_OP_CREATE,STATUS_ERR_DENIED}.
            //TODO Check if there is an existing DdsXrce::ProxyClient object associated with the same ClientKey and if
            // so compare the session_id of the existing ProxyClient with the one in the object_representation:
            // o If a ProxyClient exists and has the same session_id then the operation shall not perform any action
            // and shall set the returnValue to {STATUS_LAST_OP_CREATE,STATUS_OK}.
            // o If a ProxyClient exist and has a different session_id then the operation shall delete the existing
            // DdsXrce::ProxyClient object and shall proceed as if the ProxyClient did not exist.
            // Check there are sufficient internal resources to complete the create operation. If there are not, then the operation
            // shall fail and set the returnValue to {STATUS_LAST_OP_CREATE, STATUS_ERR_RESOURCES}.
            clients_[client_key] = ProxyClient{create_info.object_representation().client()};
            std::cout << "ProxyClient created: " << std::endl;
            status.result().implementation_status(STATUS_OK);
        }
        else{
            status.result().implementation_status(STATUS_ERR_INCOMPATIBLE);
        }
    }
    else
    {        
        status.result().implementation_status(STATUS_ERR_INVALID_DATA);
    }
    return status;
}

Status Agent::delete_client(int32_t client_key, const DELETE_PAYLOAD& delete_info)
{
    Status status;
    status.result().request_id(delete_info.request_id());
    status.result().status(STATUS_LAST_OP_DELETE);
    status.object_id(delete_info.object_id());
    if (0 == clients_.erase(client_key))
    {
        status.result().implementation_status(STATUS_ERR_INVALID_DATA);
    }
    else
    {
        status.result().implementation_status(STATUS_OK);
    }
    return status;
}

void Agent::run()
{
    const size_t buffer_size = 2048;
    char* test_buffer = new char[buffer_size];
    std::cout << "Running eProsima Agent. To stop execution enter \"Q\"" << std::endl;
    char ch = ' ';
    int ret = 0;
    do
    {
        if(ch == 'Q')
        {
            std::cout << "Stopping execution " << std::endl;
            abort_execution();
            if (response_thread_.get())
            {
                response_thread_->join();
            }
            break;
        }
        if (ch == 'c' || ch == 'd' || ch == 'w' || ch == 'r')
        {
            const uint32_t client_key = 0xF1F2F3F4;
            const uint8_t session_id = 0x01;
            const uint8_t stream_id = 0x04;
            const uint16_t sequence_nr = 0x0200;

            switch (ch)
            {
                case 'c':
                {
                    std::cout << "Testing creation" << std::endl;
                    Serializer serializer(test_buffer, buffer_size);
                    MessageHeader message_header;
                    message_header.client_key(client_key);
                    message_header.session_id(session_id);
                    message_header.stream_id(stream_id);
                    message_header.sequence_nr(sequence_nr);
                    OBJK_CLIENT_Representation client_representation;
                    client_representation.xrce_cookie(XRCE_COOKIE);
                    client_representation.xrce_version(XRCE_VERSION);
                    client_representation.xrce_vendor_id();
                    client_representation.client_timestamp();
                    client_representation.session_id();
                    ObjectVariant variant;
                    variant.client(client_representation);                    
                    const RequestId request_id = { 0x01,0x02 };
                    const ObjectId object_id = { 0xC0,0xB0,0xA0 };
                    CREATE_PAYLOAD create_data;
                    create_data.request_id(request_id);
                    create_data.object_id(object_id);
                    create_data.object_representation().client(client_representation);

                    SubmessageHeader submessage_header;
                    submessage_header.submessage_id(CREATE);
                    submessage_header.submessage_length(create_data.getCdrSerializedSize(create_data));

                    serializer.serialize(message_header);
                    serializer.serialize(submessage_header);
                    serializer.serialize(create_data);

                    XRCEParser myParser{test_buffer, serializer.get_serialized_size(), this};
                    myParser.parse();
                }
                case 'd':
                case 'w':
                case 'r':
                default:
                break;
            }
        }
        else if (ch)
        {
            std::cout << "Command " << ch << " not recognized, please enter to stop execution enter \"Q\":" << std::endl;
        }
        // if (0 < (ret = receive(in_buffer, buffer_len, loc.kind, ch_id)))
        // {
        //     printf("RECV: %d bytes\n", ret);
        //     XRCEParser myParser{reinterpret_cast<char*>(in_buffer), ret, this};
        //     myParser.parse();
        // }
        // else
        // {
        //     printf("RECV ERROR: %d\n", ret);
        // }

    }while(std::cin >> ch);
    std::cout << "Execution stopped" << std::endl;
    delete[] test_buffer;
}

void Agent::abort_execution()
{
    response_control_.running_ = false;
    messages_.abort();
}

void Agent::add_reply(const Message& message)
{
    messages_.push(message);
    if(response_thread_.get() == nullptr)
    {
        response_control_.running_ = true;
        response_thread_.reset(new std::thread(std::bind(&Agent::reply, this)));
    }
}


void Agent::add_reply(const MessageHeader& header, const Status& status_reply)
{
    Message message{};
    XRCEFactory message_creator{ reinterpret_cast<char*>(message.get_buffer().data()), message.get_buffer().max_size() };
    message_creator.header(header);
    message_creator.status(status_reply);
    add_reply(message);
}
void Agent::add_reply(const MessageHeader& header, const DATA_PAYLOAD& data)
{
    Message message{};
    XRCEFactory message_creator{ reinterpret_cast<char*>(message.get_buffer().data()), message.get_buffer().max_size() };
    message_creator.header(header);
    message_creator.data(data);
    add_reply(message);
}

void Agent::reply()
{
    std::cout << "Reply thread started. Id: " << std::this_thread::get_id() << std::endl;
    while(response_control_.running_)
    {
        Message message = messages_.pop();
        if (!message.get_buffer().empty())
        {
            Serializer deserializer(reinterpret_cast<char*>(message.get_buffer().data()), message.get_buffer().size());
            MessageHeader deserialized_header;
            SubmessageHeader deserialized_submessage_header;
            // RESOURCE_STATUS_PAYLOAD deserialized_status_payload;
            Status deserialized_status;
            deserializer.deserialize(deserialized_header);
            deserializer.deserialize(deserialized_submessage_header);
            deserializer.deserialize(deserialized_status);
            std::cout << deserialized_header << std::endl;
            std::cout << deserialized_submessage_header << std::endl;
            std::cout << deserialized_status << std::endl;
            // int ret = 0;
            // if (0 < (ret = send(message.get_buffer().data(), message.get_buffer().size(), loc.kind, ch_id)))
            // {
            //     printf("SEND: %d bytes\n", ret);
            // }
            // else
            // {
            //     printf("SEND ERROR: %d\n", ret);
            // }
        }
    }
    std::cout << "Stoping Reply thread Id: " << std::this_thread::get_id() << std::endl;
}



void Agent::on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const CREATE_PAYLOAD& create_payload)
{
    if (create_payload.object_representation().discriminator() != OBJK_CLIENT)
    {
        if (ProxyClient* client = get_client(header.client_key()))
        {
            CreationMode creation_mode;
            creation_mode.reuse(false);
            creation_mode.replace(true);

            // TODO get sub_header flags
            // Bit 1, the ‘Reuse’ bit, encodes the value of the CreationMode reuse field.
            // Bit 2, the ‘Replace’ bit, encodes the value of the CreationMode replace field.
            Status result_status = client->create(creation_mode, create_payload);
            add_reply(header, result_status);
        }
        else
        {
            std::cerr << "Create message rejected" << std::endl;
            // TODO Cuando el cliente no existe
        }
    }
    else if (create_payload.object_representation().discriminator() == OBJK_CLIENT)
    {
        Status result_status = create_client(header.client_key(), create_payload);
        add_reply(header, result_status);
    }
}

void Agent::on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const DELETE_PAYLOAD& delete_payload)
{
    if (ProxyClient* client = get_client(header.client_key()))
    {
        Status result_status = client->delete_object(delete_payload);
        add_reply(header, result_status);
    }
    else
    {
        std::cerr << "Write message rejected" << std::endl;
    }
}

void Agent::on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const WRITE_DATA_PAYLOAD&  write_payload)
{
    if (ProxyClient* client = get_client(header.client_key()))
    {
        Status result_status = client->write(write_payload.object_id(), write_payload);
        add_reply(header, result_status);
    }
    else
    {
        std::cerr << "Write message rejected" << std::endl;
    }
}

void Agent::on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const READ_DATA_PAYLOAD&   read_payload)
{
    if (ProxyClient* client = get_client(header.client_key()))
    {
        Status result_status = client->read(read_payload.object_id(), read_payload);
        add_reply(header, result_status);
    }
    else
    {
        std::cerr << "Read message rejected" << std::endl;
    }
} 

ProxyClient* Agent::get_client(int32_t client_key)
{
    try
    {
        return &clients_.at(client_key);
    } catch (const std::out_of_range& e)
    {
        std::cerr << "Client " << client_key << "not found" << std::endl;
        return nullptr;
    }
}
