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

namespace eprosima {
namespace micrortps {

void Processor::process_input_message(const XrceMessage& input_message, uint32_t addr, uint16_t port)
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
                    process_create_client_submessage(header, deserializer, addr, port);
                }
            }
        }
        /* Process the rest of the messages. */
        else
        {
            ProxyClient* client = nullptr;
            client = (header.session_id() < 128) ? root_.get_client(header.client_key()) : root_.get_client(addr);

            if (nullptr != client)
            {
                Session& session = client->session();
                dds::xrce::StreamId stream_id = header.stream_id();
                uint16_t seq_num = header.sequence_nr();
                XrceMessage message{deserializer.get_current_position(), deserializer.get_remainder_size()};
                if (session.next_input_message(stream_id, seq_num, message))
                {
                    /* Process message. */
                    process_message(header, deserializer, *client);
                    client = (header.session_id() < 128) ? root_.get_client(header.client_key()) : root_.get_client(addr);
                    while (nullptr != client && session.pop_input_message(stream_id, message))
                    {
                        Serializer temp_deserializer(message.buf, message.len);
                        process_message(header, temp_deserializer, *client);
                        session.remove_last_message(stream_id);
                        client = (header.session_id() < 128) ? root_.get_client(header.client_key()) : root_.get_client(addr);
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

void Processor::process_message(const dds::xrce::MessageHeader& header, Serializer& deserializer, ProxyClient& client)
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
                process_create_submessage(header, sub_header, deserializer, client);
                break;
            case dds::xrce::GET_INFO:
                /* TODO (Julian). */
                break;
            case dds::xrce::DELETE_ID:
                process_delete_submessage(header, sub_header, deserializer, client);
                break;
            case dds::xrce::WRITE_DATA:
                process_write_data_submessage(header, sub_header, deserializer, client);
                break;
            case dds::xrce::READ_DATA:
                process_read_data_submessage(header, sub_header, deserializer, client);
                break;
            case dds::xrce::HEARTBEAT:
                process_heartbeat_submessage(header, sub_header, deserializer, client);
                break;
            case dds::xrce::ACKNACK:
                process_acknack_submessage(header, sub_header, deserializer, client);
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

bool Processor::process_create_client_submessage(dds::xrce::MessageHeader& header, Serializer& deserializer, uint32_t addr, uint16_t port)
{
    bool rv = true;
    dds::xrce::CREATE_CLIENT_Payload client_payload;
    if (deserializer.deserialize(client_payload))
    {
        dds::xrce::STATUS_AGENT_Payload status_payload;
        status_payload.related_request().request_id(client_payload.request_id());
        status_payload.related_request().object_id(client_payload.object_id());
        dds::xrce::AGENT_Representation agent_representation;
        status_payload.result(root_.create_client(client_payload.client_representation(), agent_representation, addr, port));
        status_payload.agent_info(agent_representation);

        /* Send STATUS_AGENT submessage. */
        Message status_message{};
        XRCEFactory message_creator{status_message.get_buffer().data(), status_message.get_buffer().max_size()};
        dds::xrce::MessageHeader output_header = header;
        output_header.session_id(client_payload.client_representation().session_id());
        message_creator.header(output_header);
        message_creator.status_agent(status_payload);
        status_message.set_real_size(message_creator.get_total_size());
        status_message.set_addr(addr);
        status_message.set_port(port);
        root_.add_reply(status_message);
    }
    else
    {
        std::cerr << "Error processing CREATE_CLIENT submessage." << std::endl;
        rv = false;
    }
    return rv;
}

bool Processor::process_delete_client_submessage(dds::xrce::MessageHeader& header, Serializer& deserializer, uint32_t addr, uint16_t port)
{
    bool rv = true;
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
            client_key = root_.get_key(addr);
        }
        status_payload.result(root_.delete_client(client_key));

        Message status_message{};
        XRCEFactory message_creator{status_message.get_buffer().data(), status_message.get_buffer().max_size()};
        message_creator.header(header);
        message_creator.status(status_payload);
        status_message.set_real_size(message_creator.get_total_size());
        status_message.set_addr(addr);
        status_message.set_port(port);
        root_.add_reply(status_message);
    }
    else
    {
        std::cerr << "Error processing DELETE submessage." << std::endl;
        rv = false;
    }
    return rv;
}

bool Processor::process_create_submessage(const dds::xrce::MessageHeader& header, const dds::xrce::SubmessageHeader& sub_header, Serializer& deserializer, ProxyClient& client)
{
    bool rv = true;
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
        uint16_t seq_num = client.session().next_ouput_message(stream_id);
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
        client.session().push_output_message(stream_id, {message.get_buffer().data(), message.get_real_size()});

        /* Send status. */
        root_.add_reply(message);
    }
    else
    {
        std::cerr << "Error processing CREATE submessage." << std::endl;
        rv = false;
    }
    return rv;
}

bool Processor::process_delete_submessage(const dds::xrce::MessageHeader& header, const dds::xrce::SubmessageHeader& /*sub_header*/, Serializer& deserializer, ProxyClient& client)
{
    bool rv = true;
    dds::xrce::DELETE_Payload payload;
    if (deserializer.deserialize(payload))
    {
        /* Status message header. */
        dds::xrce::MessageHeader status_header;
        status_header.session_id(header.session_id());
        status_header.client_key(header.client_key());

        /* Status payload. */
        dds::xrce::STATUS_Payload status_payload;
        status_payload.related_request().request_id(payload.request_id());
        status_payload.related_request().object_id(payload.object_id());

        /* Serialize status. */
        Message message{};
        XRCEFactory message_creator{message.get_buffer().data(), message.get_buffer().max_size()};
        message_creator.header(status_header);
        message_creator.status(status_payload);
        message.set_real_size(message_creator.get_total_size());
        message.set_addr(client.get_addr());
        message.set_port(client.get_port());

        /* Delete object. */
        if ((payload.object_id().at(1) & 0x0F) == dds::xrce::OBJK_CLIENT)
        {
            /* Set stream and sequence number. */
            status_header.sequence_nr(0x00);
            status_header.stream_id(0x00);

            /* Set result status. */
            dds::xrce::ClientKey client_key;
            if (header.session_id() < 128)
            {
                client_key = header.client_key();
            }
            else
            {
                client_key = root_.get_key(client.get_addr());
            }
            status_payload.result(root_.delete_client(client_key));
        }
        else
        {
            /* Set stream and sequence number. */
            uint8_t stream_id = 0x80;
            uint16_t seq_num = client.session().next_ouput_message(stream_id);
            status_header.sequence_nr(seq_num);
            status_header.stream_id(stream_id);

            /* Set result status. */
            status_payload.result(client.delete_object(payload.object_id()));

            /* Store message. */
            client.session().push_output_message(stream_id, {message.get_buffer().data(), message.get_real_size()});
        }

        /* Send status. */
        root_.add_reply(message);
    }
    else
    {
        std::cerr << "Error processing DELETE submessage." << std::endl;
        rv = false;
    }
    return rv;
}

bool Processor::process_write_data_submessage(const dds::xrce::MessageHeader&, const dds::xrce::SubmessageHeader& sub_header, Serializer& deserializer, ProxyClient& client)
{
    bool rv = true;
    bool deserialized = false;
    uint8_t flags = sub_header.flags() & 0x0E;
    dds::xrce::DataRepresentation data;
    switch (flags)
    {
        case dds::xrce::FORMAT_DATA_FLAG: ;
        {
            dds::xrce::WRITE_DATA_Payload_Data payload;
            if (deserializer.deserialize(payload))
            {
                DataWriter* data_writer = dynamic_cast<DataWriter*>(client.get_object(payload.object_id()));
                if (nullptr != data_writer)
                {
                    data_writer->write(payload);
                }
                deserialized = true;
            }
            break;
        }
        default:
            break;
    }

    if (!deserialized)
    {
        std::cerr << "Error processing WRITE_DATA submessage." << std::endl;
        rv = false;
    }
    return rv;
}

bool Processor::process_read_data_submessage(const dds::xrce::MessageHeader& header, const dds::xrce::SubmessageHeader&, Serializer& deserializer, ProxyClient& client)
{
    bool rv = true;
    dds::xrce::READ_DATA_Payload payload;
    if (deserializer.deserialize(payload))
    {
        /* Status message header. */
        dds::xrce::MessageHeader status_header;
        status_header.session_id(header.session_id());
        status_header.stream_id(header.stream_id());
        uint16_t seq_num = client.stream_manager().next_ouput_message(header.stream_id());
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
        client.stream_manager().store_output_message(header.stream_id(), message.get_buffer().data(), message.get_real_size());

        /* Send status. */
        root_.add_reply(message);
    }
    else
    {
        std::cerr << "Error processing READ_DATA submessage." << std::endl;
        rv = false;
    }
    return rv;
}

bool Processor::process_acknack_submessage(const dds::xrce::MessageHeader& header, const dds::xrce::SubmessageHeader&, Serializer& deserializer, ProxyClient& client)
{
    bool rv = true;
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
                message = client.session().get_output_message((uint8_t) header.sequence_nr(), first_message + i);
                if (message.len != 0)
                {
                    Message output_message(message.buf, message.len);
                    output_message.set_addr(client.get_addr());
                    output_message.set_port(client.get_port());
                    root_.add_reply(output_message);
                }
            }
            if ((nack_bitmap.at(0) & mask) == mask)
            {
                message = client.session().get_output_message((uint8_t) header.sequence_nr(), first_message + i + 8);
                if (message.len != 0)
                {
                    Message output_message(message.buf, message.len);
                    output_message.set_addr(client.get_addr());
                    output_message.set_port(client.get_port());
                    root_.add_reply(output_message);
                }
            }
        }

        /* Update output stream. */
        client.session().update_from_acknack((uint8_t) header.sequence_nr(), first_message);
    }
    else
    {
        std::cerr << "Error processing ACKNACK submessage." << std::endl;
        rv = false;
    }
    return rv;
}

bool Processor::process_heartbeat_submessage(const dds::xrce::MessageHeader& header, const dds::xrce::SubmessageHeader&, Serializer& deserializer, ProxyClient& client)
{
    bool rv = true;
    dds::xrce::HEARTBEAT_Payload payload;
    if (deserializer.deserialize(payload))
    {
        /* Update input stream. */
        dds::xrce::StreamId stream_id = static_cast<dds::xrce::StreamId>(header.sequence_nr());
        client.session().update_from_heartbeat(stream_id,
                                               payload.first_unacked_seq_nr(),
                                               payload.last_unacked_seq_nr());

        /* Send ACKNACK message. */
        dds::xrce::MessageHeader acknack_header;
        acknack_header.session_id() = header.session_id();
        acknack_header.stream_id() = 0x00;
        acknack_header.sequence_nr() = header.sequence_nr();
        acknack_header.client_key() = header.client_key();

        dds::xrce::ACKNACK_Payload acknack_payload;
        acknack_payload.first_unacked_seq_num(client.session().get_first_unacked_seq_num(stream_id));
        acknack_payload.nack_bitmap(client.session().get_nack_bitmap(stream_id));

        Message message{};
        XRCEFactory message_creator{message.get_buffer().data(), message.get_buffer().max_size()};
        message_creator.header(acknack_header);
        message_creator.acknack(acknack_payload);
        message.set_real_size(message_creator.get_total_size());
        message.set_addr(client.get_addr());
        message.set_port(client.get_port());
        root_.add_reply(message);
    }
    else
    {
        std::cerr << "Error procession HEARTBEAT submessage." << std::endl;
        rv = false;
    }
    return rv;
}

} // namespace micrortps
} // namespace eprosima
