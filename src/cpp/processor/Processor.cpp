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

#include <uxr/agent/processor/Processor.hpp>
#include <uxr/agent/datawriter/DataWriter.hpp>
#include <uxr/agent/datareader/DataReader.hpp>
#include <uxr/agent/Root.hpp>
#include <uxr/agent/transport/Server.hpp>
#include <uxr/agent/utils/Time.hpp>

namespace eprosima {
namespace uxr {

Processor::Processor(
        Server* server,
        Middleware::Kind middleware_kind)
    : server_(server)
    , middleware_kind_{middleware_kind}
    , root_(Root::instance())
{}

Processor::~Processor()
{
}

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
        dds::xrce::ClientKey client_key = (128 > header.session_id()) ?
                                          header.client_key() :
                                          server_->get_client_key(input_packet.source.get());
        std::shared_ptr<ProxyClient> client = root_.get_client(client_key);
        if (nullptr != client)
        {
            /* Check whether it is the next message. */
            Session& session = client->session();
            dds::xrce::StreamId stream_id = input_packet.message->get_header().stream_id();
            dds::xrce::SequenceNr sequence_nr = input_packet.message->get_header().sequence_nr();
            session.push_input_message(std::move(input_packet.message), stream_id, sequence_nr);
            while (session.pop_input_message(stream_id, input_packet.message))
            {
                process_input_message(*client, input_packet);
            }

            /* Send acknack in case. */
            if (is_reliable_stream(stream_id))
            {
                /* ACKNACK header. */
                dds::xrce::MessageHeader acknack_header;
                acknack_header.session_id(header.session_id());
                acknack_header.stream_id(dds::xrce::STREAMID_NONE);
                acknack_header.sequence_nr(0x00);
                acknack_header.client_key(header.client_key());

                /* ACKNACK payload. */
                dds::xrce::ACKNACK_Payload acknack_payload;
                client->session().fill_acknack(stream_id, acknack_payload);
                acknack_payload.stream_id(header.stream_id());

                /* ACKNACK subheader. */
                dds::xrce::SubmessageHeader acknack_subheader;
                acknack_subheader.submessage_id(dds::xrce::ACKNACK);
                acknack_subheader.flags(dds::xrce::FLAG_LITTLE_ENDIANNESS);
                acknack_subheader.submessage_length(uint16_t(acknack_payload.getCdrSerializedSize()));

                /* Compute message size. */
                const size_t message_size = acknack_header.getCdrSerializedSize() +
                                            acknack_subheader.getCdrSerializedSize() +
                                            acknack_payload.getCdrSerializedSize();

                /* Set output packet and serialize ACKNACK. */
                OutputPacket output_packet;
                output_packet.destination = input_packet.source;
                output_packet.message.reset(new OutputMessage(acknack_header, message_size));
                output_packet.message->append_submessage(dds::xrce::ACKNACK, acknack_payload);

                /* Send message. */
                server_->push_output_packet(output_packet);
            }
        }
    }
}

void Processor::process_input_message(
        ProxyClient& client,
        InputPacket& input_packet)
{
    while (input_packet.message->prepare_next_submessage() && process_submessage(client, input_packet))
    {
    }
}

bool Processor::process_submessage(ProxyClient& client, InputPacket& input_packet)
{
    bool rv;
    dds::xrce::SubmessageId submessage_id = input_packet.message->get_subheader().submessage_id();
    mtx_.lock();
    switch (submessage_id)
    {
        case dds::xrce::CREATE_CLIENT:
            rv = process_create_client_submessage(input_packet);
            break;
        case dds::xrce::CREATE:
            rv = process_create_submessage(client, input_packet);
            break;
        case dds::xrce::GET_INFO:
            // TODO (julian): implement get info functionality.
            rv = false;
            break;
        case dds::xrce::DELETE_ID:
            rv = process_delete_submessage(client, input_packet);
            break;
        case dds::xrce::WRITE_DATA:
            rv = process_write_data_submessage(client, input_packet);
            break;
        case dds::xrce::READ_DATA:
            rv = process_read_data_submessage(client, input_packet);
            break;
        case dds::xrce::ACKNACK:
            rv = process_acknack_submessage(client, input_packet);
            break;
        case dds::xrce::HEARTBEAT:
            rv = process_heartbeat_submessage(client, input_packet);
            break;
        case dds::xrce::RESET:
            rv = process_reset_submessage(client, input_packet);
            break;
        case dds::xrce::FRAGMENT:
            rv = process_fragment_submessage(client, input_packet);
            break;
        case dds::xrce::TIMESTAMP:
            rv = process_timestamp_submessage(client, input_packet);
            break;
//        case dds::xrce::PERFORMANCE:
//            rv = process_performance_submessage(client, input_packet);
//            break;
        default:
            rv = false;
            break;
    }
    mtx_.unlock();
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
        /* Check whether there is a client associate with the source. */
        dds::xrce::ClientKey client_key = server_->get_client_key(input_packet.source.get());
        if ((dds::xrce::CLIENTKEY_INVALID != client_key) &&
            (client_payload.client_representation().client_key() != client_key))
        {
            dds::xrce::StatusValue delete_status = root_.delete_client(client_key).status();
            if ((dds::xrce::STATUS_OK == delete_status) ||
                (dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE == delete_status))
            {
                server_->on_delete_client(input_packet.source.get());
            }
            else
            {
                rv = false;
            }
        }

        /* Create client in case. */
        if (rv)
        {
            /* STATUS_AGENT header. */
            dds::xrce::MessageHeader status_header = input_packet.message->get_header();
            status_header.session_id(client_payload.client_representation().session_id());

            /* Create client. */
            dds::xrce::AGENT_Representation agent_representation;
            dds::xrce::ResultStatus result = root_.create_client(
                        client_payload.client_representation(),
                        agent_representation,
                        middleware_kind_);

            if (dds::xrce::STATUS_OK == result.status())
            {
                server_->on_create_client(input_packet.source.get(),
                                          client_payload.client_representation());
            }
            /* STATUS_AGENT payload. */
            dds::xrce::STATUS_AGENT_Payload status_agent;
            status_agent.result(result);
            status_agent.agent_info(agent_representation);

            /* STATUS_AGENT subheader. */
            dds::xrce::SubmessageHeader status_subheader;
            status_subheader.submessage_id(dds::xrce::STATUS_AGENT);
            status_subheader.flags(dds::xrce::FLAG_LITTLE_ENDIANNESS);
            status_subheader.submessage_length(uint16_t(status_agent.getCdrSerializedSize()));

            /* Compute message size. */
            const size_t message_size = status_header.getCdrSerializedSize() +
                                        status_subheader.getCdrSerializedSize() +
                                        status_agent.getCdrSerializedSize();

            /* Set output packet and serialize STATUS_AGENT. */
            OutputPacket output_packet;
            output_packet.destination = input_packet.source;
            output_packet.message = std::shared_ptr<OutputMessage>(new OutputMessage(status_header, message_size));
            output_packet.message->append_submessage(dds::xrce::STATUS_AGENT, status_agent);

            /* Send message. */
            server_->push_output_packet(output_packet);
        }
    }
    else
    {
        rv = false;
    }

    return rv;
}

bool Processor::process_create_submessage(
        ProxyClient& client,
        InputPacket& input_packet)
{
    bool rv = true;
    dds::xrce::CreationMode creation_mode;
    creation_mode.reuse(0 < (input_packet.message->get_subheader().flags() & dds::xrce::FLAG_REUSE));
    creation_mode.replace(0 < (input_packet.message->get_subheader().flags() & dds::xrce::FLAG_REPLACE));

    dds::xrce::CREATE_Payload create_payload;
    if (input_packet.message->get_payload(create_payload))
    {
        /* STATUS payload. */
        dds::xrce::STATUS_Payload status_payload;
        status_payload.related_request().request_id(create_payload.request_id());
        status_payload.related_request().object_id(create_payload.object_id());
        status_payload.result(client.create_object(creation_mode,
                                                   create_payload.object_id(),
                                                   create_payload.object_representation()));

        /* Push submessage into the output stream. */
        client.session().push_output_submessage(dds::xrce::STREAMID_BUILTIN_RELIABLE, dds::xrce::STATUS, status_payload);

        /* Set output packet. */
        OutputPacket output_packet;
        output_packet.destination = input_packet.source;
        while (client.session().get_next_output_message(dds::xrce::STREAMID_BUILTIN_RELIABLE, output_packet.message))
        {
            /* Send status. */
            server_->push_output_packet(output_packet);
        }
    }
    return rv;
}

bool Processor::process_delete_submessage(
        ProxyClient& client,
        InputPacket& input_packet)
{
    bool rv = true;
    dds::xrce::DELETE_Payload delete_payload;
    if (input_packet.message->get_payload(delete_payload))
    {
        /* STATUS payload. */
        dds::xrce::STATUS_Payload status_payload;
        status_payload.related_request().request_id(delete_payload.request_id());
        status_payload.related_request().object_id(delete_payload.object_id());

        /* Serialize STATUS. */
        OutputPacket output_packet;
        output_packet.destination = input_packet.source;

        /* Delete object. */
        if ((delete_payload.object_id().at(1) & 0x0F) == dds::xrce::OBJK_CLIENT)
        {
            /* Set result status. */
            dds::xrce::ClientKey client_key = client.get_client_key();
            status_payload.result(root_.delete_client(client_key));
            if (dds::xrce::STATUS_OK == status_payload.result().status())
            {
                server_->on_delete_client(input_packet.source.get());
            }
            client.session().push_output_submessage(dds::xrce::STREAMID_NONE, dds::xrce::STATUS, status_payload);
            if (client.session().get_next_output_message(dds::xrce::STREAMID_NONE, output_packet.message))
            {
                /* Send message. */
                server_->push_output_packet(output_packet);
            }
        }
        else
        {
            /* Set result status. */
            status_payload.result(client.delete_object(delete_payload.object_id()));

            /* Store message. */
            client.session().push_output_submessage(dds::xrce::STREAMID_BUILTIN_RELIABLE, dds::xrce::STATUS, status_payload);
            while (client.session().get_next_output_message(dds::xrce::STREAMID_BUILTIN_RELIABLE, output_packet.message))
            {
                /* Send message. */
                server_->push_output_packet(output_packet);
            }
        }
    }
    else
    {
        std::cerr << "Error processing DELETE submessage." << std::endl;
        rv = false;
    }
    return rv;
}

bool Processor::process_write_data_submessage(
        ProxyClient& client,
        InputPacket& input_packet)
{
    bool rv = true;
    bool deserialized = false, written = false;
    uint8_t flags = input_packet.message->get_subheader().flags() & 0x0E;
    uint16_t submessage_length = input_packet.message->get_subheader().submessage_length();
    switch (flags)
    {
        case dds::xrce::FORMAT_DATA_FLAG:
        {
            dds::xrce::WRITE_DATA_Payload_Data data_payload;
            data_payload.data().resize(submessage_length - data_payload.BaseObjectRequest::getCdrSerializedSize(0));
            if (input_packet.message->get_payload(data_payload))
            {
                std::shared_ptr<DataWriter> data_writer =
                        std::dynamic_pointer_cast<DataWriter>(client.get_object(data_payload.object_id()));
                if (nullptr != data_writer)
                {
                    written = data_writer->write(data_payload);
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

    if(!written)
    {
        std::cerr << "Error written data on DDS entity." << std::endl;
        rv = false;
    }

    return rv;
}

bool Processor::process_read_data_submessage(
        ProxyClient& client,
        InputPacket& input_packet)
{
    bool rv = true;
    dds::xrce::READ_DATA_Payload read_payload;
    if (input_packet.message->get_payload(read_payload))
    {
        std::shared_ptr<DataReader> data_reader =
                std::dynamic_pointer_cast<DataReader>(client.get_object(read_payload.object_id()));
        dds::xrce::StatusValue status = (nullptr != data_reader) ? dds::xrce::STATUS_OK
                                                                 : dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE;
        if (dds::xrce::STATUS_OK == status)
        {
            /* Set callback args. */
            ReadCallbackArgs cb_args;
            cb_args.client_key = client.get_client_key();
            cb_args.stream_id = read_payload.read_specification().preferred_stream_id();
            cb_args.object_id = read_payload.object_id();
            cb_args.request_id = read_payload.request_id();

            /* Launch read data. */
            using namespace std::placeholders;
            if (!data_reader->read(read_payload, std::bind(&Processor::read_data_callback, this, _1, _2), cb_args))
            {
                status = dds::xrce::STATUS_ERR_RESOURCES;
            }
        }

        if (dds::xrce::STATUS_OK != status)
        {
            /* STATUS payload. */
            dds::xrce::STATUS_Payload status_payload;
            status_payload.related_request().request_id(read_payload.request_id());
            status_payload.related_request().object_id(read_payload.object_id());
            status_payload.result().implementation_status(0x00);
            status_payload.result().status(status);

            /* Push submessage into the output stream. */
            client.session().push_output_submessage(dds::xrce::STREAMID_BUILTIN_RELIABLE, dds::xrce::STATUS, status_payload);

            /* Set output packet. */
            OutputPacket output_packet;
            output_packet.destination = input_packet.source;
            while (client.session().get_next_output_message(dds::xrce::STREAMID_BUILTIN_RELIABLE, output_packet.message))
            {
                /* Send message. */
                server_->push_output_packet(output_packet);
            }
        }
    }
    else
    {
        std::cerr << "Error processing READ_DATA submessage." << std::endl;
        rv = false;
    }
    return rv;
}

bool Processor::process_acknack_submessage(
        ProxyClient& client,
        InputPacket& input_packet)
{
    bool rv = true;
    dds::xrce::ACKNACK_Payload acknack_payload;
    if (input_packet.message->get_payload(acknack_payload))
    {
        /* Send missing messages again. */
        uint16_t first_message = acknack_payload.first_unacked_seq_num();
        std::array<uint8_t, 2> nack_bitmap = acknack_payload.nack_bitmap();
        uint8_t stream_id = acknack_payload.stream_id();
        for (uint16_t i = 0; i < 8; ++i)
        {
            OutputPacket output_packet;
            output_packet.destination = input_packet.source;
            uint8_t mask = uint8_t(0x01 << i);
            if ((nack_bitmap.at(1) & mask) == mask)
            {
                if (client.session().get_output_message(stream_id, first_message + i, output_packet.message))
                {
                    server_->push_output_packet(output_packet);
                }
            }
            if ((nack_bitmap.at(0) & mask) == mask)
            {
                if (client.session().get_output_message(stream_id, first_message + i + 8, output_packet.message))
                {
                    server_->push_output_packet(output_packet);
                }
            }
        }

        /* Update output stream. */
        client.session().update_from_acknack(stream_id, first_message);
    }
    else
    {
        std::cerr << "Error processing ACKNACK submessage." << std::endl;
        rv = false;
    }
    return rv;
}

bool Processor::process_heartbeat_submessage(
        ProxyClient& client,
        InputPacket& input_packet)
{
    bool rv = true;
    dds::xrce::HEARTBEAT_Payload heartbeat_payload;
    if (input_packet.message->get_payload(heartbeat_payload))
    {
        /* Update input stream. */
        uint8_t stream_id = heartbeat_payload.stream_id();
        client.session().update_from_heartbeat(stream_id,
                                               heartbeat_payload.first_unacked_seq_nr(),
                                               heartbeat_payload.last_unacked_seq_nr());

        /* ACKNACK payload. */
        dds::xrce::ACKNACK_Payload acknack_payload;
        client.session().fill_acknack(stream_id, acknack_payload);
        acknack_payload.stream_id(stream_id);

        /* Push submessage into the output stream. */
        client.session().push_output_submessage(dds::xrce::STREAMID_NONE, dds::xrce::ACKNACK, acknack_payload);

        /* Set output packet. */
        OutputPacket output_packet;
        output_packet.destination = input_packet.source;
        if (client.session().get_next_output_message(dds::xrce::STREAMID_NONE, output_packet.message))
        {
            /* Send message. */
            server_->push_output_packet(output_packet);
        }
    }
    else
    {
        std::cerr << "Error procession HEARTBEAT submessage." << std::endl;
        rv = false;
    }
    return rv;
}

bool Processor::process_reset_submessage(
        ProxyClient& client,
        InputPacket& /*input_packet*/)
{
    client.session().reset();
    return true;
}

bool Processor::process_fragment_submessage(
        ProxyClient& client,
        InputPacket& input_packet)
{
    dds::xrce::StreamId stream_id = input_packet.message->get_header().stream_id();
    client.session().push_input_fragment(stream_id, input_packet.message);
    InputPacket fragment_packet;
    if (client.session().pop_input_fragment_message(stream_id, fragment_packet.message))
    {
        fragment_packet.source = input_packet.source;
        process_input_message(client, fragment_packet);
    }
    return true;
}

bool Processor::process_timestamp_submessage(ProxyClient& client, InputPacket& input_packet)
{
    bool rv = true;
    dds::xrce::TIMESTAMP_Payload timestamp;
    if (input_packet.message->get_payload(timestamp))
    {
        dds::xrce::TIMESTAMP_REPLY_Payload timestamp_reply;
        time::get_epoch_time(timestamp_reply.receive_timestamp().seconds(),
                             timestamp_reply.receive_timestamp().nanoseconds());
        timestamp_reply.originate_timestamp(timestamp.transmit_timestamp());
        time::get_epoch_time(timestamp_reply.transmit_timestamp().seconds(),
                             timestamp_reply.transmit_timestamp().nanoseconds());

        client.session().push_output_submessage(dds::xrce::STREAMID_NONE, dds::xrce::TIMESTAMP_REPLY, timestamp_reply);

        OutputPacket output_packet;
        output_packet.destination = input_packet.source;
        if (client.session().get_next_output_message(dds::xrce::STREAMID_NONE, output_packet.message))
        {
            server_->push_output_packet(output_packet);
        }
    }
    else
    {
        std::cerr << "Error processin TIMESTAMP submessage." << std::endl;
        rv = false;
    }
    return rv;
}

//bool Processor::process_performance_submessage(ProxyClient& client, InputPacket& input_packet)
//{
//    /* Set output packet. */
//    OutputPacket output_packet;
//    output_packet.destination = input_packet.source;
//
//    /* Get epoch time and array. */
//    uint8_t buf[UINT16_MAX];
//    uint16_t submessage_len = input_packet.message->get_subheader().submessage_length();
//    input_packet.message->get_raw_payload(buf, size_t(submessage_len));
//
//    /* Check ECHO. */
//    if (dds::xrce::FLAG_ECHO == (dds::xrce::FLAG_ECHO & input_packet.message->get_subheader().flags()))
//    {
//        /* PERFORMANCE header. */
//        dds::xrce::MessageHeader output_header;
//        output_header.session_id(input_packet.message->get_header().session_id());
//        output_header.stream_id(input_packet.message->get_header().stream_id());
//        output_header.sequence_nr(client.session().next_output_message(output_header.stream_id()));
//        output_header.client_key(input_packet.message->get_header().client_key());
//
//        /* PERFORMANCE subheader. */
//        dds::xrce::SubmessageHeader performance_subheader;
//        performance_subheader.submessage_id(dds::xrce::PERFORMANCE);
//        performance_subheader.flags(0x01);
//        performance_subheader.submessage_length(submessage_len);
//
//        const size_t message_size = output_header.getCdrSerializedSize() +
//                                    performance_subheader.getCdrSerializedSize() +
//                                    submessage_len;
//
//        /* Generate output packect. */
//        output_packet.message = OutputMessagePtr(new OutputMessage(output_header, message_size));
//        output_packet.message->append_raw_payload(dds::xrce::PERFORMANCE, buf, size_t(submessage_len));
//        if (client.session().push_output_message(output_header.stream_id(), output_packet.message))
//        {
//            /* Send message. */
//            server_->push_output_packet(output_packet);
//        }
//    }
//    return true;
//}

void Processor::read_data_callback(
        const ReadCallbackArgs& cb_args,
        const std::vector<uint8_t>& buffer)
{
    mtx_.lock();
    std::shared_ptr<ProxyClient> client = root_.get_client(cb_args.client_key);

    /* DATA payload. */
    dds::xrce::DATA_Payload_Data data_payload;
    data_payload.request_id(cb_args.request_id);
    data_payload.object_id(cb_args.object_id);
    data_payload.data().serialized_data(buffer);

    /* Set output packet and serialize DATA. */
    OutputPacket output_packet;
    output_packet.destination = server_->get_source(cb_args.client_key);
    if (output_packet.destination)
    {
        /* Push submessage into the output stream. */
        client->session().push_output_submessage(cb_args.stream_id, dds::xrce::DATA, data_payload);

        /* Set output message. */
        while (client->session().get_next_output_message(cb_args.stream_id, output_packet.message))
        {
            /* Send message. */
            server_->push_output_packet(output_packet);
        }
    }
    mtx_.unlock();
}

bool Processor::process_get_info_packet(
        InputPacket&& input_packet,
        dds::xrce::TransportAddress& address,
        OutputPacket& output_packet) const
{
    bool rv = false;

    if (input_packet.message->prepare_next_submessage())
    {
        if (input_packet.message->get_subheader().submessage_id() == dds::xrce::GET_INFO)
        {
            /* Get GET_INFO payload. */
            dds::xrce::GET_INFO_Payload get_info_payload;
            input_packet.message->get_payload(get_info_payload);

            /* Get info from root. */
            dds::xrce::ObjectInfo object_info;
            dds::xrce::ResultStatus result_status = root_.get_info(object_info);
            if (dds::xrce::STATUS_OK == result_status.status())
            {
                dds::xrce::AGENT_ActivityInfo agent_info;
                agent_info.address_seq().push_back(address);
                agent_info.availability(1);

                dds::xrce::ActivityInfoVariant info_variant;
                info_variant.agent(agent_info);
                object_info.activity(info_variant);

                /* INFO payload. */
                dds::xrce::INFO_Payload info_payload;
                info_payload.related_request().request_id(get_info_payload.request_id());
                info_payload.related_request().object_id(get_info_payload.object_id());
                info_payload.result(result_status);
                info_payload.object_info(object_info);

                /* INFO subheader. */
                dds::xrce::SubmessageHeader info_subheader;
                info_subheader.submessage_id(dds::xrce::INFO);
                info_subheader.flags(dds::xrce::FLAG_LITTLE_ENDIANNESS);
                info_subheader.submessage_length(uint16_t(info_payload.getCdrSerializedSize()));

                /* Compute message size. */
                const size_t message_size = input_packet.message->get_header().getCdrSerializedSize() +
                                            info_subheader.getCdrSerializedSize() +
                                            info_payload.getCdrSerializedSize();

                /* Set output packet and serialize INFO. */
                output_packet.destination = input_packet.source;
                output_packet.message = OutputMessagePtr(new OutputMessage(input_packet.message->get_header(),
                                                                           message_size));
                rv = output_packet.message->append_submessage(dds::xrce::INFO, info_payload);
            }
        }
    }

    return rv;
}

void Processor::check_heartbeats()
{
    /* HEARTBEAT header. */
    dds::xrce::MessageHeader header;
    header.stream_id(dds::xrce::STREAMID_NONE);
    header.sequence_nr(0x00);

    /* HEARTBEAT payload. */
    dds::xrce::HEARTBEAT_Payload heartbeat;

    /* HEARTBEAT subheader. */
    dds::xrce::SubmessageHeader subheader;
    subheader.submessage_id(dds::xrce::HEARTBEAT);
    subheader.flags(dds::xrce::FLAG_LITTLE_ENDIANNESS);
    subheader.submessage_length(uint16_t(heartbeat.getCdrSerializedSize()));

    const size_t message_size =
            header.getCdrSerializedSize() +
            subheader.getCdrSerializedSize() +
            heartbeat.getCdrSerializedSize();

    OutputPacket output_packet;

    std::shared_ptr<ProxyClient> client;
    while (root_.get_next_client(client))
    {
        if ((output_packet.destination = server_->get_source(client->get_client_key())))
        {
            header.session_id(client->get_session_id());
            header.client_key(client->get_client_key());

            /* Get reliable streams. */
            for (auto stream : client->session().get_output_streams())
            {
                /* Get and send pending messages. */
                if (client->session().fill_heartbeat(stream, heartbeat))
                {
                    output_packet.message = OutputMessagePtr(new OutputMessage(header, message_size));
                    output_packet.message->append_submessage(dds::xrce::HEARTBEAT, heartbeat);

                    /* Send message. */
                    server_->push_output_packet(output_packet);
                }
            }
        }
    }
}

} // namespace uxr
} // namespace eprosima
