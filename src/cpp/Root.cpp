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

#include <MessageHeader.h>
#include <SubMessageHeader.h>
// #include <libdev/MessageDebugger.h>
#include <types/Shape.h>

#include <fastcdr/Cdr.h>

#include <memory>

using eprosima::micrortps::Agent;
using eprosima::micrortps::ResultStatus;
//using eprosima::micrortps::debug::operator<<;


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
    response_thread_{},
    response_control_{}    
{
    response_control_.running_ = false;
    response_control_.run_scheduled_ = false;
}

void Agent::init()
{
    std::cout << "Agent initialization..." << std::endl;
    // Init transport
    loc_ = locator_t{LOC_SERIAL, "/dev/ttyACM0"};
    ch_id_ = add_locator(&loc_);
}

void Agent::demo_create_client()
{
    ClientKey client_key = {{0xF1, 0xF2, 0xF3, 0xF4}};
    
    OBJK_CLIENT_Representation client_representation;
    client_representation.xrce_cookie({XRCE_COOKIE});
    client_representation.xrce_version({XRCE_VERSION});
    client_representation.xrce_vendor_id();
    client_representation.client_timestamp();
    client_representation.session_id();
    ObjectVariant variant;
    variant.client(client_representation);
    
    const RequestId request_id{ {1,2} };
    const ObjectId object_id{ {10,20} };
    CREATE_Payload create_data;
    create_data.object_id(object_id);
    create_data.object_representation().client(client_representation);

    MessageHeader message_header;
    message_header.client_key(client_key);
    message_header.session_id(1);
    message_header.stream_id(2);
    message_header.sequence_nr(0);

    ResultStatus st = create_client(message_header, create_data);
    if (st.implementation_status() == STATUS_OK)
    {
        //debug::short_print(std::cout, "    INTERNAL [Create | id: 0xF1F2F3F4 | 0001 OK | CREATE_CLIENT]\n");
    }
}

ResultStatus Agent::create_client(const MessageHeader& header, const CREATE_Payload& create_info)
{
    ResultStatus status;
    status.request_id(create_info.request_id());
    status.status(STATUS_LAST_OP_CREATE);

    if ((create_info.object_representation()._d() == OBJK_CLIENT) && 
        (create_info.object_representation().client().xrce_cookie() == XRCE_COOKIE))
    {
        if (create_info.object_representation().client().xrce_version()[0] <= XRCE_VERSION_MAJOR)
        {
            // TODO(borja): The Agent shall check the ClientKey to ensure it is authorized to connect to the Agent
            // If this check fails the operation shall fail and returnValue is set to {STATUS_LAST_OP_CREATE,STATUS_ERR_DENIED}.
            // TODO(borja): Check if there is an existing DdsXrce::ProxyClient object associated with the same ClientKey and if
            // so compare the session_id of the existing ProxyClient with the one in the object_representation:
            // o If a ProxyClient exists and has the same session_id then the operation shall not perform any action
            // and shall set the returnValue to {STATUS_LAST_OP_CREATE,STATUS_OK}.
            // o If a ProxyClient exist and has a different session_id then the operation shall delete the existing
            // DdsXrce::ProxyClient object and shall proceed as if the ProxyClient did not exist.
            // Check there are sufficient internal resources to complete the create operation. If there are not, then the operation
            // shall fail and set the returnValue to {STATUS_LAST_OP_CREATE, STATUS_ERR_RESOURCES}.
            clients_[header.client_key()] = ProxyClient{create_info.object_representation().client(), header};
            client_ids_[create_info.object_id()] = header.client_key();
            //std::cout << "ProxyClient created " << std::endl;
            status.implementation_status(STATUS_OK);
        }
        else{
            status.implementation_status(STATUS_ERR_INCOMPATIBLE);
        }
    }
    else
    {        
        status.implementation_status(STATUS_ERR_INVALID_DATA);
    }
    return status;
}

ResultStatus Agent::delete_client(ClientKey client_key, const DELETE_RESOURCE_Payload& delete_info)
{
    ResultStatus status;
    status.request_id(delete_info.request_id());
    status.status(STATUS_LAST_OP_DELETE);
    if ((0 ==clients_.erase(client_key)) || (0 == client_ids_.erase(delete_info.object_id())))
    {
        status.implementation_status(STATUS_ERR_INVALID_DATA);
    }
    else
    {
        status.implementation_status(STATUS_OK);
    }
    return status;
}

void Agent::demo_message_create(char* test_buffer, size_t buffer_size)
{
    ClientKey client_key = {{0xF1, 0xF2, 0xF3, 0xF4}};
    const uint8_t session_id = 0x01;
    const uint8_t stream_id = 0x04;
    const uint16_t sequence_nr = 0x0200;
    std::cout << "Testing creation" << std::endl;
    Serializer serializer(test_buffer, buffer_size);
    MessageHeader message_header;
    message_header.client_key(client_key);
    message_header.session_id(session_id);
    message_header.stream_id(stream_id);
    message_header.sequence_nr(sequence_nr);
    OBJK_CLIENT_Representation client_representation;
    client_representation.xrce_cookie({XRCE_COOKIE});
    client_representation.xrce_version({XRCE_VERSION});
    client_representation.xrce_vendor_id();
    client_representation.client_timestamp();
    client_representation.session_id();
    ObjectVariant variant;
    variant.client(client_representation);                    
    const RequestId request_id{ {0x01,0x02} };
    const ObjectId object_id{ {0xC0,0xB0} };
    CREATE_Payload create_data;
    create_data.request_id(request_id);
    create_data.object_id(object_id);
    create_data.object_representation().client(client_representation);

    SubmessageHeader submessage_header;
    submessage_header.submessage_id(CREATE);
    submessage_header.submessage_length(create_data.getCdrSerializedSize());

    serializer.serialize(message_header);
    serializer.serialize(submessage_header);
    serializer.serialize(create_data);

    XRCEParser myParser{test_buffer, serializer.get_serialized_size(), this};
    myParser.parse();
}

void Agent::demo_message_subscriber(char* test_buffer, size_t buffer_size)
{
    ClientKey client_key = {{0xF1, 0xF2, 0xF3, 0xF4}};
    const uint8_t session_id = 0x01;
    const uint8_t stream_id = 0x04;
    const uint16_t sequence_nr = 0x0200;
    Serializer serializer(test_buffer, buffer_size);
    MessageHeader message_header;
    message_header.client_key(client_key);
    message_header.session_id(session_id);
    message_header.stream_id(stream_id);
    message_header.sequence_nr(sequence_nr);

    const RequestId request_id{ {0x01,0x02} };

    ObjectVariant variant;
    OBJK_SUBSCRIBER_Representation subs;
    subs.representation().object_reference(std::string("SUBSCRIBER"));
    subs.participant_id({ {4,4} });

    CREATE_Payload create_data;
    create_data.request_id(request_id);
    create_data.object_id({ {10,20} });
    create_data.object_representation().subscriber(subs);

    SubmessageHeader submessage_header;
    submessage_header.submessage_id(CREATE);
    submessage_header.submessage_length(create_data.getCdrSerializedSize());

    serializer.serialize(message_header);
    serializer.serialize(submessage_header);
    serializer.serialize(create_data);

    XRCEParser myParser{test_buffer, serializer.get_serialized_size(), this};
    myParser.parse();
}

void Agent::demo_delete_subscriber(char* test_buffer, size_t buffer_size)
{
    ClientKey client_key = {{0xF1, 0xF2, 0xF3, 0xF4}};
    const uint8_t session_id = 0x01;
    const uint8_t stream_id = 0x04;
    const uint16_t sequence_nr = 0x0200;
    Serializer serializer(test_buffer, buffer_size);
    MessageHeader message_header;
    message_header.client_key(client_key);
    message_header.session_id(session_id);
    message_header.stream_id(stream_id);
    message_header.sequence_nr(sequence_nr);

    const RequestId request_id = { {0x01,0x02} };

    CREATE_Payload create_data;
    create_data.request_id(request_id);
    create_data.object_id({ {10,20} });

    SubmessageHeader submessage_header;
    submessage_header.submessage_id(CREATE);
    submessage_header.submessage_length(create_data.getCdrSerializedSize());

    serializer.serialize(message_header);
    serializer.serialize(submessage_header);
    serializer.serialize(create_data);

    XRCEParser myParser{test_buffer, serializer.get_serialized_size(), this};
    myParser.parse();
}

void demo_message_publisher(char*  /*test_buffer*/, size_t  /*buffer_size*/)
{

}

void Agent::demo_message_read(char * test_buffer, size_t buffer_size)
{
    ClientKey client_key = {{0xF1, 0xF2, 0xF3, 0xF4}};
    const uint8_t session_id = 0x01;
    const uint8_t stream_id = 0x04;
    const uint16_t sequence_nr = 0x0200;
    std::cout << "Testing read" << std::endl;
    Serializer serializer(test_buffer, buffer_size);
    MessageHeader message_header;
    message_header.client_key(client_key);
    message_header.session_id(session_id);
    message_header.stream_id(stream_id);
    message_header.sequence_nr(sequence_nr);
    READ_DATA_Payload read_payload;
    read_payload.request_id();
    read_payload.object_id({ {10,20} });
    read_payload.read_specification();
    SubmessageHeader submessage_header;
    submessage_header.submessage_id(READ_DATA);
    submessage_header.submessage_length(read_payload.getCdrSerializedSize());

    serializer.serialize(message_header);
    serializer.serialize(submessage_header);
    serializer.serialize(read_payload);

    XRCEParser myParser{test_buffer, serializer.get_serialized_size(), this};
    myParser.parse();
}

void Agent::demo_message_publisher(char* test_buffer, size_t buffer_size)
{
    ClientKey client_key = {{0xF1, 0xF2, 0xF3, 0xF4}};
    const uint8_t session_id = 0x01;
    const uint8_t stream_id = 0x04;
    const uint16_t sequence_nr = 0x0200;
    std::cout << "Testing creation" << std::endl;
    Serializer serializer(test_buffer, buffer_size);
    MessageHeader message_header;
    message_header.client_key(client_key);
    message_header.session_id(session_id);
    message_header.stream_id(stream_id);
    message_header.sequence_nr(sequence_nr);

    const RequestId request_id = { {0x01,0x02} };

    ObjectVariant variant;
    OBJK_PUBLISHER_Representation pubs;
    pubs.representation().object_reference(std::string("PUBLISHER"));
    pubs.participant_id({ {4,4} });

    CREATE_Payload create_data;
    create_data.request_id(request_id);
    create_data.object_id({ {10,20} });
    create_data.object_representation().publisher(pubs);

    SubmessageHeader submessage_header;
    submessage_header.submessage_id(CREATE);
    submessage_header.submessage_length(create_data.getCdrSerializedSize());

    serializer.serialize(message_header);
    serializer.serialize(submessage_header);
    serializer.serialize(create_data);

    XRCEParser myParser{test_buffer, serializer.get_serialized_size(), this};
    myParser.parse();
}

void Agent::demo_message_write(char * test_buffer, size_t buffer_size)
{
    static int x = 0;
    static int y = 0;
    ++x; ++y;

    ClientKey client_key = {{0xF1, 0xF2, 0xF3, 0xF4}};
    const uint8_t session_id = 0x01;
    const uint8_t stream_id = 0x04;
    const uint16_t sequence_nr = 0x0200;
    std::cout << "Testing read" << std::endl;
    Serializer serializer(test_buffer, buffer_size);
    MessageHeader message_header;
    message_header.client_key(client_key);
    message_header.session_id(session_id);
    message_header.stream_id(stream_id);
    message_header.sequence_nr(sequence_nr);
    WRITE_DATA_Payload write_payload;
    write_payload.request_id();
    write_payload.object_id({ {10,20} });

    // Serialize data
    ShapeType st;
    st.color("RED");
    st.x(x);
    st.y(y);
    st.shapesize(10);
    eprosima::fastcdr::FastBuffer fbuffer;
    eprosima::fastcdr::Cdr ser(fbuffer);
    ser.serialize(st);
    std::vector<unsigned char> buffer(ser.getBufferPointer(), ser.getBufferPointer() + ser.getSerializedDataLength());
    write_payload.data_to_write().data().serialized_data(buffer);

    SubmessageHeader submessage_header;
    submessage_header.submessage_id(WRITE_DATA);
    submessage_header.submessage_length(write_payload.getCdrSerializedSize());

    serializer.serialize(message_header);
    serializer.serialize(submessage_header);
    serializer.serialize(write_payload);

    XRCEParser myParser{test_buffer, serializer.get_serialized_size(), this};
    myParser.parse();
}

void Agent::demo_process_response(Message& message)
{
    Serializer deserializer(message.get_buffer().data(), message.get_real_size());
    MessageHeader deserialized_header;
    SubmessageHeader deserialized_submessage_header;

    deserializer.deserialize(deserialized_header);
    deserializer.deserialize(deserialized_submessage_header);

    switch (deserialized_submessage_header.submessage_id())
    {
        case STATUS:
        {
            RESOURCE_STATUS_Payload deserialized_status;
            deserializer.deserialize(deserialized_status);
            std::cout << "<== ";
            //debug::short_print(std::cout, deserialized_status, debug::STREAM_COLOR::YELLOW) << std::endl;
            break;
        }
        case DATA:
        {
            DATA_Payload_Data deserialized_data;
            deserializer.deserialize(deserialized_data);
            std::cout << "<== ";
            //debug::short_print(std::cout, deserialized_data, debug::STREAM_COLOR::YELLOW) << std::endl;
            // printf("%X\n", deserialized_data.data_reader().data().serialized_data().data());
            // ShapeType* shape = (ShapeType*)deserialized_data.data_reader().data().serialized_data().data();
            // std::cout << "<SHAPE TYPE>" << std::endl;
            // std::cout << " - color: " << shape->color().data() << std::endl;
            // std::cout << " - x: " << shape->x() << std::endl;
            // std::cout << " - y: " << shape->y() << std::endl;
            break;
        }
    }
}

void Agent::run()
{
    std::cout << "Running DDS-XRCE Agent..." << std::endl;
    int ret = 0;
    do
    {
        if (0 < (ret = receive_data(static_cast<octet*>(in_buffer_), buffer_len_, loc_.kind, ch_id_)))
        {
            //printf("RECV: %d bytes\n", ret);
            /*for (int i = 0; i < ret; ++i)
            {
                printf("%X ", in_buffer_[i]);
            }
            printf("\n");*/
            XRCEParser myParser{reinterpret_cast<char*>(in_buffer_), static_cast<size_t>(ret), this};
            myParser.parse();
        }
        usleep(1000000);

    }while(true);
    std::cout << "Execution stopped" << std::endl;
}

void Agent::abort_execution()
{
    response_control_.running_ = false;
    messages_.abort();
}

void Agent::add_reply(const Message& message)
{
    messages_.push(message);
    if(response_thread_ == nullptr)
    {
        response_control_.running_ = true;
        response_thread_.reset(new std::thread(std::bind(&Agent::reply, this)));
        // = std::make_unique<std::thread>(std::bind(&Agent::reply, this));
    }
}


void Agent::add_reply(const MessageHeader& header, const RESOURCE_STATUS_Payload& status_reply)
{
    MessageHeader updated_header{header};
    update_header(updated_header);
    Message message{};
    XRCEFactory message_creator{ message.get_buffer().data(), message.get_buffer().max_size() };
    message_creator.header(updated_header);
    message_creator.status(status_reply);
    message.set_real_size(message_creator.get_total_size());
    add_reply(message);
}

void Agent::add_reply(const MessageHeader &header, const DATA_Payload_Data &payload)
{
    MessageHeader updated_header{header};
    update_header(updated_header);
    Message message{};
    XRCEFactory message_creator{message.get_buffer().data(), message.get_buffer().max_size()};
    message_creator.header(updated_header);
    message_creator.data(payload);
    message.set_real_size(message_creator.get_total_size());
    add_reply(message);
}

void Agent::add_reply(const MessageHeader &header, const DATA_Payload_Sample &payload)
{
    MessageHeader updated_header{header};
    update_header(updated_header);
    Message message{};
    XRCEFactory message_creator{message.get_buffer().data(), message.get_buffer().max_size()};
    message_creator.header(updated_header);
    message_creator.data(payload);
    message.set_real_size(message_creator.get_total_size());
    add_reply(message);
}

void Agent::add_reply(const MessageHeader &header, const DATA_Payload_DataSeq &payload)
{
    MessageHeader updated_header{header};
    update_header(updated_header);
    Message message{};
    XRCEFactory message_creator{message.get_buffer().data(), message.get_buffer().max_size()};
    message_creator.header(updated_header);
    message_creator.data(payload);
    message.set_real_size(message_creator.get_total_size());
    add_reply(message);
}

void Agent::add_reply(const MessageHeader &header, const DATA_Payload_SampleSeq &payload)
{
    MessageHeader updated_header{header};
    update_header(updated_header);
    Message message{};
    XRCEFactory message_creator{message.get_buffer().data(), message.get_buffer().max_size()};
    message_creator.header(updated_header);
    message_creator.data(payload);
    message.set_real_size(message_creator.get_total_size());
    add_reply(message);
}

void Agent::add_reply(const MessageHeader &header, const DATA_Payload_PackedSamples &payload)
{
    MessageHeader updated_header{header};
    update_header(updated_header);
    Message message{};
    XRCEFactory message_creator{message.get_buffer().data(), message.get_buffer().max_size()};
    message_creator.header(updated_header);
    message_creator.data(payload);
    message.set_real_size(message_creator.get_total_size());
    add_reply(message);
}

void Agent::reply()
{
    //std::cout << "Reply thread started. Id: " << std::this_thread::get_id() << std::endl;
    while(response_control_.running_)
    {
        Message message = messages_.pop();
        if (!message.get_buffer().empty())
        {
            demo_process_response(message);
            int ret = 0;
            if (0 < (ret = send_data(reinterpret_cast<octet*>(message.get_buffer().data()), message.get_real_size(), loc_.kind, ch_id_)))
            {
                printf("SEND: %d bytes of %d\n", ret, message.get_buffer().size());
                for (int i = 0; i < ret; ++i)
                {
                    printf("%02X ", (unsigned char)message.get_buffer()[i]);
                }
                printf("\n");
            }
        }
        usleep(1000000);
    }
    //std::cout << "Stoping Reply thread Id: " << std::this_thread::get_id() << std::endl;
}



void Agent::on_message(const MessageHeader& header, const SubmessageHeader&  sub_header, const CREATE_Payload& create_payload)
{
    std::cout << "==> ";
    //debug::short_print(std::cout, create_payload, debug::STREAM_COLOR::GREEN) << std::endl;
    if (create_payload.object_representation()._d() != OBJK_CLIENT)
    {
        if (ProxyClient* client = get_client(header.client_key()))
        {
            CreationMode creation_mode;
            creation_mode.reuse(false);
            creation_mode.replace(true);

            // TODO(borja): get sub_header flags
            // Bit 1, the ‘Reuse’ bit, encodes the value of the CreationMode reuse field.
            // Bit 2, the ‘Replace’ bit, encodes the value of the CreationMode replace field.
            RESOURCE_STATUS_Payload status;
            status.object_id(create_payload.object_id());
            status.request_id(create_payload.request_id());
            ResultStatus result_status = client->create(creation_mode, create_payload);
            status.result(result_status);
            add_reply(header, status);
        }
        else
        {
            //debug::ColorStream cs(std::cerr, debug::STREAM_COLOR::RED);
            std::cerr << "Create message rejected" << std::endl;
        }
    }
    else if (create_payload.object_representation()._d() == OBJK_CLIENT)
    {
        RESOURCE_STATUS_Payload status;
        status.object_id(create_payload.object_id());
        status.request_id(create_payload.request_id());
        ResultStatus result_status = create_client(header, create_payload);
        add_reply(header, status);
    }
}

void Agent::on_message(const MessageHeader& header, const SubmessageHeader&  /*sub_header*/, const DELETE_RESOURCE_Payload& delete_payload)
{
    std::cout << "==> ";
    //debug::short_print(std::cout, delete_payload, debug::STREAM_COLOR::GREEN) << std::endl;
    auto client_id = client_ids_.find(delete_payload.object_id());
    if (client_id != client_ids_.end())
    {
        RESOURCE_STATUS_Payload status;
        status.request_id(delete_payload.request_id());
        status.object_id(delete_payload.object_id());
        status.result(delete_client(client_ids_.at(delete_payload.object_id()), delete_payload));
        add_reply(header, status);
    }
    else if (ProxyClient* client = get_client(header.client_key()))
    {
        RESOURCE_STATUS_Payload status;
        status.request_id(delete_payload.request_id());
        status.object_id(delete_payload.object_id());        
        status.result(client->delete_object(delete_payload));
        add_reply(header, status);
    }
    else
    {
        //debug::ColorStream cs(std::cerr, debug::STREAM_COLOR::RED);
        std::cerr << "Write message rejected" << std::endl;
    }
}

void Agent::on_message(const MessageHeader& header, const SubmessageHeader&  /*sub_header*/, const WRITE_DATA_Payload&  write_payload)
{
    std::cout << "==> ";
    //debug::short_print(std::cout, write_payload, debug::STREAM_COLOR::GREEN) << std::endl;
    if (ProxyClient* client = get_client(header.client_key()))
    {
        RESOURCE_STATUS_Payload status;
        status.request_id(write_payload.request_id());
        status.object_id(write_payload.object_id());
        ResultStatus result_status = client->write(write_payload.object_id(), write_payload);
        status.result(result_status);
        add_reply(header, status);
    }
    else
    {
        //debug::ColorStream cs(std::cerr, debug::STREAM_COLOR::RED);
        std::cerr << "Write message rejected" << std::endl;
    }
}

void Agent::on_message(const MessageHeader &header, const SubmessageHeader & /*sub_header*/,
                       const READ_DATA_Payload &read_payload)
{
    std::cout << "==> ";
    // debug::short_print(std::cout, read_payload, debug::STREAM_COLOR::GREEN) << std::endl;
    if(ProxyClient *client = get_client(header.client_key()))
    {
        RESOURCE_STATUS_Payload status;
        status.request_id(read_payload.request_id());
        status.object_id(read_payload.object_id());
        ResultStatus result_status = client->read(read_payload.object_id(), read_payload);
        status.result(result_status);
        add_reply(header, status);
    }
    else
    {
        //debug::ColorStream cs(std::cerr, debug::STREAM_COLOR::RED);
        std::cerr << "Read message rejected" << std::endl;
    }
}

eprosima::micrortps::ProxyClient* Agent::get_client(ClientKey client_key)
{
    try
    {
        return &clients_.at(client_key);
    } catch (const std::out_of_range& e)
    {
        // debug::ColorStream cs(std::cerr, debug::STREAM_COLOR::RED);
        // std::cerr << "Client " << client_key << "not found" << std::endl;
        return nullptr;
    }
}

void Agent::update_header(MessageHeader& header)
{
    // TODO(borja): sequence number is general and not independent for each client
    static uint8_t sequence = 0;
    header.sequence_nr(sequence++);
}
