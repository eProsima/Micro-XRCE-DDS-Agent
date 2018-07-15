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

#include <micrortps/agent/processor/Processor.hpp>
#include <micrortps/agent/Root.hpp>
#include <micrortps/agent/transport/Server.hpp>

namespace eprosima {
namespace micrortps {

void Processor::process_input_packet(InputPacket&& input_packet)
{
    /* Create client message. */
    if ((input_packet.message->get_header().session_id() == dds::xrce::SESSIONID_NONE_WITH_CLIENT_KEY) ||
        (input_packet.message->get_header().session_id() == dds::xrce::SESSIONID_NONE_WITHOUT_CLIENT_KEY))
    {
        if (input_packet.message->prepare_next_submessage())
        {
            if (input_packet.message->get_subheader().submessage_id() == dds::xrce::CREATE_CLIENT)
            {
                process_create_client_submessage(input_packet);
            }
        }
    }
    /* Process the rest of the messages. */
    else
    {
        dds::xrce::MessageHeader header = input_packet.message->get_header();
        dds::xrce::ClientKey client_key = input_packet.client_key;
        ProxyClient* client = root_.get_client(client_key);
        if (nullptr != client)
        {
            Session& session = client->session();
            dds::xrce::StreamId stream_id = input_packet.message->get_header().stream_id();
            if (session.next_input_message(input_packet.message))
            {
                /* Process message. */
                process_input_message(*client, input_packet.message);
                client = root_.get_client(client_key);
                while (nullptr != client && session.pop_input_message(stream_id, input_packet.message))
                {
                    process_input_message(*client, input_packet.message);
                    client = root_.get_client(client_key);
                }

            }

            /* Send acknack in case. */
            if (127 < stream_id)
            {
                dds::xrce::MessageHeader acknack_header;
                acknack_header.session_id(header.session_id());
                acknack_header.stream_id(0x00);
                acknack_header.sequence_nr(header.sequence_nr());
                acknack_header.client_key(header.client_key());

                dds::xrce::ACKNACK_Payload acknack_payload;
                acknack_payload.first_unacked_seq_num(client->session().get_first_unacked_seq_num(stream_id));
                acknack_payload.nack_bitmap(client->session().get_nack_bitmap(stream_id));

                OutputPacket output_packet;
                output_packet.server = input_packet.server;
                output_packet.destination.reset(new EndPoint(*input_packet.source));
                output_packet.client_key = client_key;
                output_packet.message.reset(new OutputMessage(acknack_header));
                output_packet.message->append_submessage(dds::xrce::ACKNACK, acknack_payload);

                output_packet.server->push_output_packet(output_packet);
            }
        }
        else
        {
            std::cerr << "Error client unknown." << std::endl;
        }
    }
}

void Processor::process_input_message(ProxyClient& client, InputMessagePtr& input_message)
{
    while (input_message->prepare_next_submessage() && process_submessage(client, input_message))
    {
    }
}

bool Processor::process_submessage(ProxyClient& client, InputMessagePtr& input_message)
{
    bool rv;
    dds::xrce::SubmessageId submessage_id = input_message->get_subheader().submessage_id();
    std::unique_lock<std::mutex> lock(client.get_mutex());
    switch (submessage_id)
    {
        case dds::xrce::CREATE_CLIENT:
//            rv = process_create_client_submessage(input_message);
            break;
        case dds::xrce::CREATE:
            rv = process_create_submessage(client, input_message);
            break;
        case dds::xrce::GET_INFO:
            // TODO (julian): implement get info functionality.
            rv = false;
            break;
        case dds::xrce::DELETE:
            // TODO (julian): handle delete client behaviour.
            rv = process_delete_submessage(client, input_message);
            break;
        case dds::xrce::WRITE_DATA:
            rv = process_write_data_submessage(client, input_message);
            break;
        case dds::xrce::READ_DATA:
            rv = process_read_data_submessage(client, input_message);
            break;
        case dds::xrce::ACKNACK:
            rv = process_acknack_submessage(client, input_message);
            break;
        case dds::xrce::HEARTBEAT:
            rv = process_heartbeat_submessage(client, input_message);
            break;
        case dds::xrce::RESET:
            // TODO (julian): implement reset functionality.
            rv = false;
            break;
        case dds::xrce::FRAGMENT:
            // TODO (julian): implement fragment functionality.
            rv = false;
            break;
        default:
            rv = false;
            break;
    }
    return rv;
}

bool Processor::process_create_client_submessage(InputPacket& input_packet)
{
    bool rv = true;
    dds::xrce::CREATE_CLIENT_Payload client_payload;
    if (((input_packet.message->get_header().session_id() == dds::xrce::SESSIONID_NONE_WITH_CLIENT_KEY) ||
         (input_packet.message->get_header().session_id() == dds::xrce::SESSIONID_NONE_WITHOUT_CLIENT_KEY)) &&
          input_packet.message->get_payload(client_payload))
    {
        dds::xrce::STATUS_AGENT_Payload status_payload;
        status_payload.related_request().request_id(client_payload.request_id());
        status_payload.related_request().object_id(client_payload.object_id());
        dds::xrce::AGENT_Representation agent_representation;
        status_payload.result(root_.create_client(client_payload.client_representation(), agent_representation, 0, 0));
        status_payload.agent_info(agent_representation);

        /* Send STATUS_AGENT submessage. */
        dds::xrce::MessageHeader output_header = input_packet.message->get_header();
        output_header.session_id(client_payload.client_representation().session_id());

        OutputPacket output_packet;
        output_packet.server = input_packet.server;
        output_packet.destination = input_packet.source;
        output_packet.client_key = client_payload.client_representation().client_key();
        output_packet.message = std::shared_ptr<OutputMessage>(new OutputMessage(output_header));
        output_packet.message->append_submessage(dds::xrce::STATUS_AGENT, status_payload);

        output_packet.server->push_output_packet(output_packet);
    }
    else
    {
        std::cerr << "Error processing CREATE_CLIENT submessage." << std::endl;
        rv = false;
    }
    return rv;
}

bool Processor::process_create_submessage(ProxyClient& client, InputMessagePtr& input_message)
{
    bool rv = true;
//    dds::xrce::CreationMode creation_mode;
//    creation_mode.reuse(0 < (input_message->get_subheader().flags() & dds::xrce::FLAG_REUSE));
//    creation_mode.replace(0 < (input_message->get_subheader().flags() & dds::xrce::FLAG_REPLACE));
//
//    dds::xrce::CREATE_Payload create_payload;
//    if (input_message->get_payload(create_payload))
//    {
//        /* Status message header. */
//        dds::xrce::MessageHeader status_header;
//        status_header.session_id(input_message->get_header().session_id());
//        status_header.stream_id(0x80);
//        status_header.sequence_nr(client.session().next_output_message(0x80));
//        status_header.client_key(input_message->get_header().client_key());
//
//        /* Status payload. */
//        dds::xrce::STATUS_Payload status_payload;
//        status_payload.related_request().request_id(create_payload.request_id());
//        status_payload.related_request().object_id(create_payload.object_id());
//        status_payload.result(client.create(creation_mode,
//                                            create_payload.object_id(),
//                                            create_payload.object_representation()));
//
//        /* Serialize status. */
//        OutputMessagePtr output_message(new OutputMessage(status_header));
//        output_message->append_submessage(dds::xrce::STATUS, status_payload);
//
//        /* Store message. */
//        client.session().push_output_message(0x80, output_message);
//
//        /* Send status. */
//        root_.add_reply(output_message);
//    }
    return rv;
}

bool Processor::process_delete_submessage(ProxyClient& client, InputMessagePtr& input_message)
{
    bool rv = true;
//    dds::xrce::DELETE_Payload delete_payload;
//    if (input_message->get_payload(delete_payload))
//    {
//        /* Status message header. */
//        dds::xrce::MessageHeader status_header;
//        status_header.session_id(input_message->get_header().session_id());
//        status_header.client_key(input_message->get_header().client_key());
//
//        /* Status payload. */
//        dds::xrce::STATUS_Payload status_payload;
//        status_payload.related_request().request_id(delete_payload.request_id());
//        status_payload.related_request().object_id(delete_payload.object_id());
//
//        /* Serialize status. */
//        OutputMessagePtr output_message;
//
//        /* Delete object. */
//        if ((delete_payload.object_id().at(1) & 0x0F) == dds::xrce::OBJK_CLIENT)
//        {
//            /* Set stream and sequence number. */
//            status_header.sequence_nr(0x00);
//            status_header.stream_id(0x00);
//
//            /* Set result status. */
//            dds::xrce::ClientKey client_key;
//            if (input_message->get_header().session_id() < 128)
//            {
//                client_key = input_message->get_header().client_key();
//            }
//            else
//            {
//                client_key = root_.get_key(client.get_addr());
//            }
//            status_payload.result(root_.delete_client(client_key));
//            output_message = OutputMessagePtr(new OutputMessage(status_header));
//        }
//        else
//        {
//            /* Set stream and sequence number. */
//            uint8_t stream_id = 0x80;
//            uint16_t seq_num = client.session().next_output_message(stream_id);
//            status_header.sequence_nr(seq_num);
//            status_header.stream_id(stream_id);
//
//            /* Set result status. */
//            status_payload.result(client.delete_object(delete_payload.object_id()));
//
//            /* Store message. */
//            output_message = OutputMessagePtr(new OutputMessage(status_header));
//            client.session().push_output_message(stream_id, output_message);
//        }
//
//        /* Send status. */
//        root_.add_reply(output_message);
//    }
//    else
//    {
//        std::cerr << "Error processing DELETE submessage." << std::endl;
//        rv = false;
//    }
    return rv;
}

bool Processor::process_write_data_submessage(ProxyClient& client, InputMessagePtr& input_message)
{
    bool rv = true;
//    bool deserialized = false;
//    uint8_t flags = input_message->get_subheader().flags() & 0x0E;
//    dds::xrce::DataRepresentation data;
//    switch (flags)
//    {
//        case dds::xrce::FORMAT_DATA_FLAG: ;
//        {
//            dds::xrce::WRITE_DATA_Payload_Data data_payload;
//            if (input_message->get_payload(data_payload))
//            {
//                DataWriter* data_writer = dynamic_cast<DataWriter*>(client.get_object(data_payload.object_id()));
//                if (nullptr != data_writer)
//                {
//                    data_writer->write(data_payload);
//                }
//                deserialized = true;
//            }
//            break;
//        }
//        default:
//            break;
//    }
//
//    if (!deserialized)
//    {
//        std::cerr << "Error processing WRITE_DATA submessage." << std::endl;
//        rv = false;
//    }
    return rv;
}

bool Processor::process_read_data_submessage(ProxyClient& client, InputMessagePtr& input_message)
{
    bool rv = true;
    dds::xrce::READ_DATA_Payload read_payload;
    if (input_message->get_payload(read_payload))
    {
        /* Status message header. */
        dds::xrce::MessageHeader status_header;
        status_header.session_id(input_message->get_header().session_id());
        status_header.stream_id(input_message->get_header().stream_id());
        uint16_t seq_num = client.stream_manager().next_ouput_message(input_message->get_header().stream_id());
        status_header.sequence_nr(seq_num);
        status_header.client_key(input_message->get_header().client_key());

        /* Status payload. */
        dds::xrce::STATUS_Payload status_payload;
        status_payload.related_request().request_id(read_payload.request_id());
        status_payload.related_request().object_id(read_payload.object_id());
        dds::xrce::ResultStatus result;
        result.implementation_status(0x00);
        DataReader* data_reader = dynamic_cast<DataReader*>(client.get_object(read_payload.object_id()));
        if (nullptr != data_reader)
        {
            data_reader->read(read_payload, input_message->get_header().stream_id());
            result.status(dds::xrce::STATUS_OK);
        }
        else
        {
            result.status(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);

            /* Serialize status. */
            OutputMessagePtr output_message(new OutputMessage(status_header));
            output_message->append_submessage(dds::xrce::STATUS, status_payload);

            /* Store message. */
            client.stream_manager().store_output_message(header.stream_id(), message.get_buffer().data(), message.get_real_size());

            /* Send status. */
            root_.add_reply(output_message);
        }
        status_payload.result(result);
    }
    else
    {
        std::cerr << "Error processing READ_DATA submessage." << std::endl;
        rv = false;
    }
    return rv;
}

bool Processor::process_acknack_submessage(ProxyClient& client, InputMessagePtr& input_message)
{
    bool rv = true;
//    dds::xrce::ACKNACK_Payload acknack_payload;
//    if (input_message->get_payload(acknack_payload))
//    {
//        /* Send missing messages again. */
//        uint16_t first_message = acknack_payload.first_unacked_seq_num();
//        std::array<uint8_t, 2> nack_bitmap = acknack_payload.nack_bitmap();
//        dds::xrce::SequenceNr seq_num = input_message->get_header().sequence_nr();
//        for (uint16_t i = 0; i < 8; ++i)
//        {
//            OutputMessagePtr output_message;
//            uint8_t mask = 0x01 << i;
//            if ((nack_bitmap.at(1) & mask) == mask)
//            {
//                if (client.session().get_output_message((uint8_t) seq_num, first_message + i, output_message))
//                {
//                    root_.add_reply(output_message);
//                }
//            }
//            if ((nack_bitmap.at(0) & mask) == mask)
//            {
//                if (client.session().get_output_message((uint8_t) seq_num, first_message + i + 8, output_message))
//                {
//                    root_.add_reply(output_message);
//                }
//            }
//        }
//
//        /* Update output stream. */
//        client.session().update_from_acknack((uint8_t) seq_num, first_message);
//    }
//    else
//    {
//        std::cerr << "Error processing ACKNACK submessage." << std::endl;
//        rv = false;
//    }
    return rv;
}

bool Processor::process_heartbeat_submessage(ProxyClient& client, InputMessagePtr& input_message)
{
    bool rv = true;
//    dds::xrce::HEARTBEAT_Payload heartbeat_payload;
//    if (input_message->get_payload(heartbeat_payload))
//    {
//        /* Update input stream. */
//        dds::xrce::StreamId stream_id = static_cast<dds::xrce::StreamId>(input_message->get_header().sequence_nr());
//        client.session().update_from_heartbeat(stream_id,
//                                               heartbeat_payload.first_unacked_seq_nr(),
//                                               heartbeat_payload.last_unacked_seq_nr());
//
//        /* Send ACKNACK message. */
//        dds::xrce::MessageHeader acknack_header;
//        acknack_header.session_id(input_message->get_header().session_id());
//        acknack_header.stream_id(0x00);
//        acknack_header.sequence_nr(input_message->get_header().sequence_nr());
//        acknack_header.client_key(input_message->get_header().client_key());
//
//        dds::xrce::ACKNACK_Payload acknack_payload;
//        acknack_payload.first_unacked_seq_num(client.session().get_first_unacked_seq_num(stream_id));
//        acknack_payload.nack_bitmap(client.session().get_nack_bitmap(stream_id));
//
//        OutputMessagePtr output_message(new OutputMessage(acknack_header));
//        output_message->append_submessage(dds::xrce::ACKNACK, acknack_payload);
//
//        root_.add_reply(output_message);
//    }
//    else
//    {
//        std::cerr << "Error procession HEARTBEAT submessage." << std::endl;
//        rv = false;
//    }
    return rv;
}

} // namespace micrortps
} // namespace eprosima
