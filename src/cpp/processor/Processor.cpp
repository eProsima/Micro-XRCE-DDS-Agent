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

namespace eprosima {
namespace uxr {

Processor::Processor(Server* server)
    : server_(server),
      root_(new Root())
{}

Processor::~Processor()
{
    delete root_;
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
        std::shared_ptr<ProxyClient> client = root_->get_client(client_key);
        if (nullptr != client)
        {
            /* Check whether it is the next message. */
            Session& session = client->session();
            dds::xrce::StreamId stream_id = input_packet.message->get_header().stream_id();
            if (session.next_input_message(input_packet.message))
            {
                /* Process messages. */
                process_input_message(*client, input_packet);
                client = root_->get_client(client_key);
                while (nullptr != client && session.pop_input_message(stream_id, input_packet.message))
                {
                    process_input_message(*client, input_packet);
                    client = root_->get_client(client_key);
                }

            }

            /* Send acknack in case. */
            if (127 < stream_id)
            {
                /* ACKNACK header. */
                dds::xrce::MessageHeader acknack_header;
                acknack_header.session_id(header.session_id());
                acknack_header.stream_id(0x00);
                acknack_header.sequence_nr(header.stream_id());
                acknack_header.client_key(header.client_key());

                /* ACKNACK payload. */
                dds::xrce::ACKNACK_Payload acknack_payload;
                acknack_payload.first_unacked_seq_num(client->session().get_first_unacked_seq_num(stream_id));
                acknack_payload.nack_bitmap(client->session().get_nack_bitmap(stream_id));

                /* Set output packet and serialize ACKNACK. */
                OutputPacket output_packet;
                output_packet.destination = input_packet.source;
                output_packet.message.reset(new OutputMessage(acknack_header));
                output_packet.message->append_submessage(dds::xrce::ACKNACK, acknack_payload);

                /* Send message. */
                server_->push_output_packet(output_packet);
            }
        }
        else
        {
            std::cerr << "Error client unknown." << std::endl;
        }
    }
}

void Processor::process_input_message(ProxyClient& client, InputPacket& input_packet)
{
    while (input_packet.message->prepare_next_submessage() && process_submessage(client, input_packet))
    {
    }
}

bool Processor::process_submessage(ProxyClient& client, InputPacket& input_packet)
{
    bool rv;
    dds::xrce::SubmessageId submessage_id = input_packet.message->get_subheader().submessage_id();
    std::lock_guard<std::mutex> lock(mtx_);
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
        /* Check whether there is a client associate with the source. */
        dds::xrce::ClientKey client_key = server_->get_client_key(input_packet.source.get());
        if ((dds::xrce::CLIENTKEY_INVALID != client_key) &&
            (client_payload.client_representation().client_key() != client_key))
        {
            dds::xrce::StatusValue delete_status = root_->delete_client(client_key).status();
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
            dds::xrce::MessageHeader output_header = input_packet.message->get_header();
            output_header.session_id(client_payload.client_representation().session_id());

            /* STATUS_AGENT payload. */
            dds::xrce::STATUS_AGENT_Payload status_payload;
            status_payload.related_request().request_id(client_payload.request_id());
            status_payload.related_request().object_id(client_payload.object_id());

            /* Create client. */
            dds::xrce::AGENT_Representation agent_representation;
            dds::xrce::ResultStatus result = root_->create_client(client_payload.client_representation(),
                                                                 agent_representation);
            if (dds::xrce::STATUS_OK == result.status())
            {
                server_->on_create_client(input_packet.source.get(),
                                          client_payload.client_representation());
            }
            status_payload.result(result);
            status_payload.agent_info(agent_representation);

            /* Set output packet and serialize STATUS_AGENT. */
            OutputPacket output_packet;
            output_packet.destination = input_packet.source;
            output_packet.message = std::shared_ptr<OutputMessage>(new OutputMessage(output_header));
            output_packet.message->append_submessage(dds::xrce::STATUS_AGENT, status_payload);

            /* Send message. */
            server_->push_output_packet(output_packet);
        }
    }
    else
    {
        std::cerr << "Error processing CREATE_CLIENT submessage." << std::endl;
        rv = false;
    }
    return rv;
}

bool Processor::process_create_submessage(ProxyClient& client, InputPacket& input_packet)
{
    bool rv = true;
    dds::xrce::CreationMode creation_mode;
    creation_mode.reuse(0 < (input_packet.message->get_subheader().flags() & dds::xrce::FLAG_REUSE));
    creation_mode.replace(0 < (input_packet.message->get_subheader().flags() & dds::xrce::FLAG_REPLACE));

    dds::xrce::CREATE_Payload create_payload;
    if (input_packet.message->get_payload(create_payload))
    {
        /* STATUS header. */
        dds::xrce::MessageHeader status_header;
        status_header.session_id(input_packet.message->get_header().session_id());
        status_header.stream_id(0x80);
        status_header.sequence_nr(client.session().next_output_message(0x80));
        status_header.client_key(input_packet.message->get_header().client_key());

        /* STATUS payload. */
        dds::xrce::STATUS_Payload status_payload;
        status_payload.related_request().request_id(create_payload.request_id());
        status_payload.related_request().object_id(create_payload.object_id());
        status_payload.result(client.create(creation_mode,
                                            create_payload.object_id(),
                                            create_payload.object_representation()));

        /* Set output packet and Serialize STATUS. */
        OutputPacket output_packet;
        output_packet.destination = input_packet.source;
        output_packet.message = OutputMessagePtr(new OutputMessage(status_header));
        output_packet.message->append_submessage(dds::xrce::STATUS, status_payload);

        /* Store message. */
        client.session().push_output_message(0x80, output_packet.message);

        /* Send status. */
        server_->push_output_packet(output_packet);
    }
    return rv;
}

bool Processor::process_delete_submessage(ProxyClient& client, InputPacket& input_packet)
{
    bool rv = true;
    dds::xrce::DELETE_Payload delete_payload;
    if (input_packet.message->get_payload(delete_payload))
    {
        /* STATUS header. */
        dds::xrce::MessageHeader status_header;
        status_header.session_id(input_packet.message->get_header().session_id());
        status_header.client_key(input_packet.message->get_header().client_key());

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
            /* Set stream and sequence number. */
            status_header.sequence_nr(0x00);
            status_header.stream_id(0x00);

            /* Set result status. */
            dds::xrce::ClientKey client_key = client.get_client_key();
            status_payload.result(root_->delete_client(client_key));
            if (dds::xrce::STATUS_OK == status_payload.result().status())
            {
                server_->on_delete_client(input_packet.source.get());
            }
            output_packet.message = OutputMessagePtr(new OutputMessage(status_header));
        }
        else
        {
            /* Set stream and sequence number. */
            uint8_t stream_id = 0x80;
            uint16_t seq_num = client.session().next_output_message(stream_id);
            status_header.sequence_nr(seq_num);
            status_header.stream_id(stream_id);

            /* Set result status. */
            status_payload.result(client.delete_object(delete_payload.object_id()));

            /* Store message. */
            output_packet.message = OutputMessagePtr(new OutputMessage(status_header));
            client.session().push_output_message(stream_id, output_packet.message);
        }
        output_packet.message->append_submessage(dds::xrce::STATUS, status_payload, 0);

        /* Send message. */
        server_->push_output_packet(output_packet);
    }
    else
    {
        std::cerr << "Error processing DELETE submessage." << std::endl;
        rv = false;
    }
    return rv;
}

bool Processor::process_write_data_submessage(ProxyClient& client, InputPacket& input_packet)
{
    bool rv = true;
    bool deserialized = false, written = false;
    uint8_t flags = input_packet.message->get_subheader().flags() & 0x0E;
    dds::xrce::DataRepresentation data;
    switch (flags)
    {
        case dds::xrce::FORMAT_DATA_FLAG: ;
        {
            dds::xrce::WRITE_DATA_Payload_Data data_payload;
            if (input_packet.message->get_payload(data_payload))
            {
                deserialized = true;
                DataWriter* data_writer = dynamic_cast<DataWriter*>(client.get_object(data_payload.object_id()));
                if (nullptr != data_writer)
                {
                    written = data_writer->write(data_payload);
                }
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

bool Processor::process_read_data_submessage(ProxyClient& client, InputPacket& input_packet)
{
    bool rv = true;
    dds::xrce::READ_DATA_Payload read_payload;
    if (input_packet.message->get_payload(read_payload))
    {
        DataReader* data_reader = dynamic_cast<DataReader*>(client.get_object(read_payload.object_id()));
        if (nullptr != data_reader)
        {
            /* Set callback args. */
            ReadCallbackArgs cb_args;
            cb_args.client_key = client.get_client_key();
            cb_args.stream_id = read_payload.read_specification().data_stream_id();
            cb_args.object_id = read_payload.object_id();
            cb_args.request_id = read_payload.request_id();

            /* Launch read data. */
            using namespace std::placeholders;
            data_reader->read(read_payload, std::bind(&Processor::read_data_callback, this, _1, _2), cb_args);
        }
        else
        {
            /* STATUS header. */
            uint8_t stream_id = 0x80;
            dds::xrce::MessageHeader status_header;
            status_header.session_id(input_packet.message->get_header().session_id());
            status_header.stream_id(stream_id);
            status_header.sequence_nr(client.session().next_output_message(stream_id));
            status_header.client_key(input_packet.message->get_header().client_key());

            /* STATUS payload. */
            dds::xrce::STATUS_Payload status_payload;
            status_payload.related_request().request_id(read_payload.request_id());
            status_payload.related_request().object_id(read_payload.object_id());
            status_payload.result().implementation_status(0x00);
            status_payload.result().status(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);

            /* Set output packet and serialize STATUS. */
            OutputPacket output_packet;
            output_packet.destination = input_packet.source;
            output_packet.message = OutputMessagePtr(new OutputMessage(status_header));
            output_packet.message->append_submessage(dds::xrce::STATUS, status_payload);

            /* Store message. */
            client.session().push_output_message(stream_id, output_packet.message);

            /* Send message. */
            server_->push_output_packet(output_packet);
        }
    }
    else
    {
        std::cerr << "Error processing READ_DATA submessage." << std::endl;
        rv = false;
    }
    return rv;
}

bool Processor::process_acknack_submessage(ProxyClient& client, InputPacket& input_packet)
{
    bool rv = true;
    dds::xrce::ACKNACK_Payload acknack_payload;
    if (input_packet.message->get_payload(acknack_payload))
    {
        /* Send missing messages again. */
        uint16_t first_message = acknack_payload.first_unacked_seq_num();
        std::array<uint8_t, 2> nack_bitmap = acknack_payload.nack_bitmap();
        dds::xrce::SequenceNr seq_num = input_packet.message->get_header().sequence_nr();
        for (uint16_t i = 0; i < 8; ++i)
        {
            OutputPacket output_packet;
            output_packet.destination = input_packet.source;
            uint8_t mask = uint8_t(0x01 << i);
            if ((nack_bitmap.at(1) & mask) == mask)
            {
                if (client.session().get_output_message(uint8_t(seq_num), first_message + i, output_packet.message))
                {
                    server_->push_output_packet(output_packet);
                }
            }
            if ((nack_bitmap.at(0) & mask) == mask)
            {
                if (client.session().get_output_message(uint8_t(seq_num), first_message + i + 8, output_packet.message))
                {
                    server_->push_output_packet(output_packet);
                }
            }
        }

        /* Update output stream. */
        client.session().update_from_acknack(uint8_t(seq_num), first_message);
    }
    else
    {
        std::cerr << "Error processing ACKNACK submessage." << std::endl;
        rv = false;
    }
    return rv;
}

bool Processor::process_heartbeat_submessage(ProxyClient& client, InputPacket& input_packet)
{
    bool rv = true;
    dds::xrce::HEARTBEAT_Payload heartbeat_payload;
    if (input_packet.message->get_payload(heartbeat_payload))
    {
        /* Update input stream. */
        dds::xrce::StreamId stream_id;
        stream_id = static_cast<dds::xrce::StreamId>(input_packet.message->get_header().sequence_nr());
        client.session().update_from_heartbeat(stream_id,
                                               heartbeat_payload.first_unacked_seq_nr(),
                                               heartbeat_payload.last_unacked_seq_nr());

        /* ACKNACK header. */
        dds::xrce::MessageHeader acknack_header;
        acknack_header.session_id(input_packet.message->get_header().session_id());
        acknack_header.stream_id(0x00);
        acknack_header.sequence_nr(stream_id);
        acknack_header.client_key(input_packet.message->get_header().client_key());

        /* ACKNACK payload. */
        dds::xrce::ACKNACK_Payload acknack_payload;
        acknack_payload.first_unacked_seq_num(client.session().get_first_unacked_seq_num(stream_id));
        acknack_payload.nack_bitmap(client.session().get_nack_bitmap(stream_id));

        /* Set output packet and serialize ACKNACK. */
        OutputPacket output_packet;
        output_packet.destination = input_packet.source;
        output_packet.message = OutputMessagePtr(new OutputMessage(acknack_header));
        output_packet.message->append_submessage(dds::xrce::ACKNACK, acknack_payload);

        /* Send message. */
        server_->push_output_packet(output_packet);
    }
    else
    {
        std::cerr << "Error procession HEARTBEAT submessage." << std::endl;
        rv = false;
    }
    return rv;
}

bool Processor::process_reset_submessage(ProxyClient& client, InputPacket& /*input_packet*/)
{
    client.session().reset();
    return true;
}

void Processor::read_data_callback(const ReadCallbackArgs& cb_args, const std::vector<uint8_t>& buffer)
{
    std::lock_guard<std::mutex> lock(mtx_);
    std::shared_ptr<ProxyClient> client = root_->get_client(cb_args.client_key);

    /* DATA header. */
    dds::xrce::MessageHeader message_header;
    message_header.client_key(client->get_client_key());
    message_header.session_id(client->get_session_id());
    message_header.stream_id(cb_args.stream_id);
    message_header.sequence_nr(client->session().next_output_message(cb_args.stream_id));

    /* DATA payload. */
    dds::xrce::DATA_Payload_Data payload;
    payload.request_id(cb_args.request_id);
    payload.object_id(cb_args.object_id);
    payload.data().serialized_data(buffer);

    /* Set output packet and serialize DATA. */
    OutputPacket output_packet;
    output_packet.destination = server_->get_source(cb_args.client_key);
    if (output_packet.destination)
    {
        output_packet.message = OutputMessagePtr(new OutputMessage(message_header));
        output_packet.message->append_submessage(dds::xrce::DATA, payload, dds::xrce::FORMAT_DATA_FLAG | 0x01);

        /* Store message. */
        client->session().push_output_message(cb_args.stream_id, output_packet.message);

        /* Send message. */
        server_->push_output_packet(output_packet);
    }
}

bool Processor::process_get_info_packet(InputPacket&& input_packet,
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
            dds::xrce::ResultStatus result_status = root_->get_info(object_info);
            if (dds::xrce::STATUS_OK == result_status.status())
            {
                dds::xrce::AGENT_ActivityInfo agent_info;
                agent_info.address_seq().push_back(address);
                agent_info.availability(1);

                dds::xrce::ActivityInfoVariant info_variant;
                info_variant.agent(agent_info);
                object_info.activity(info_variant);

                dds::xrce::INFO_Payload payload;
                payload.related_request().request_id(get_info_payload.request_id());
                payload.related_request().object_id(get_info_payload.object_id());
                payload.result(result_status);
                payload.object_info(object_info);

                /* Set output packet and serialize INFO. */
                output_packet.destination = input_packet.source;
                output_packet.message = OutputMessagePtr(new OutputMessage(input_packet.message->get_header()));
                rv = output_packet.message->append_submessage(dds::xrce::INFO, payload);
            }
        }
    }

    return rv;
}

void Processor::check_heartbeats()
{
    root_->init_client_iteration();
    std::shared_ptr<ProxyClient> client;
    while (root_->get_next_client(client))
    {
        /* Get reliable streams. */
        for (auto stream : client->session().get_output_streams())
        {
            /* Get and send pending messages. */
            if (client->session().message_pending(stream))
            {
                /* Heartbeat message header. */
                dds::xrce::MessageHeader heartbeat_header;
                heartbeat_header.session_id(client->get_session_id());
                heartbeat_header.stream_id(0x00);
                heartbeat_header.sequence_nr(stream);
                heartbeat_header.client_key(client->get_client_key());

                /* Heartbeat message payload. */
                dds::xrce::HEARTBEAT_Payload heartbeat_payload;
                heartbeat_payload.first_unacked_seq_nr(client->session().get_first_unacked_seq_nr(stream));
                heartbeat_payload.last_unacked_seq_nr(client->session().get_last_unacked_seq_nr(stream));

                /* Set output packet and serialize HEARTBEAT. */
                OutputPacket output_packet;
                output_packet.destination = server_->get_source(client->get_client_key());
                if (output_packet.destination)
                {
                    output_packet.message = OutputMessagePtr(new OutputMessage(heartbeat_header));
                    output_packet.message->append_submessage(dds::xrce::HEARTBEAT, heartbeat_payload);

                    /* Send message. */
                    server_->push_output_packet(output_packet);
                }
            }
        }
    }
}

} // namespace uxr
} // namespace eprosima
