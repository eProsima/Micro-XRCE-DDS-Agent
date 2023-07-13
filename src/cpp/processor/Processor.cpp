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
#include <uxr/agent/requester/Requester.hpp>
#include <uxr/agent/replier/Replier.hpp>
#include <uxr/agent/Root.hpp>
#include <uxr/agent/transport/Server.hpp>
#include <uxr/agent/utils/Time.hpp>

#include <uxr/agent/transport/endpoint/IPv4EndPoint.hpp>
#include <uxr/agent/transport/endpoint/IPv6EndPoint.hpp>
#include <uxr/agent/transport/endpoint/CanEndPoint.hpp>
#include <uxr/agent/transport/endpoint/SerialEndPoint.hpp>
#include <uxr/agent/transport/endpoint/MultiSerialEndPoint.hpp>
#include <uxr/agent/transport/endpoint/CustomEndPoint.hpp>

namespace eprosima {
namespace uxr {

template<typename EndPoint>
Processor<EndPoint>::Processor(
        Server<EndPoint>& server,
        Root& root,
        Middleware::Kind middleware_kind)
    : server_(server)
    , middleware_kind_{middleware_kind}
    , root_(root)
{}

template<typename EndPoint>
void Processor<EndPoint>::process_input_packet(
        InputPacket<EndPoint>&& input_packet)
{
    dds::xrce::MessageHeader header = input_packet.message->get_header();

    if ((header.session_id() == dds::xrce::SESSIONID_NONE_WITH_CLIENT_KEY) ||
        (header.session_id() == dds::xrce::SESSIONID_NONE_WITHOUT_CLIENT_KEY))
    {
        if (input_packet.message->prepare_next_submessage())
        {
            switch (input_packet.message->get_subheader().submessage_id())
            {
                case dds::xrce::CREATE_CLIENT:
                {
                    process_create_client_submessage(input_packet);
                    break;
                }
                // Handle out of session ping from client (known or unknown)
                case dds::xrce::GET_INFO:
                {
                    OutputPacket<EndPoint> output_packet;
                    process_get_info_packet(std::move(input_packet), output_packet);
                    server_.push_output_packet(std::move(output_packet));
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
    }
    else
    {
        dds::xrce::ClientKey client_key = dds::xrce::CLIENTKEY_INVALID;
        if (128 > header.session_id())
        {
            client_key = header.client_key();
        }
        else
        {
            uint32_t raw_client_key;
            if (server_.get_client_key(input_packet.source, raw_client_key))
            {
                client_key = conversion::raw_to_clientkey(raw_client_key);
            }
        }
        std::shared_ptr<ProxyClient> client = root_.get_client(client_key);

        if (client)
        {
            client->update_state();

            Session& session = client->session();
            dds::xrce::StreamId stream_id = input_packet.message->get_header().stream_id();
            dds::xrce::SequenceNr sequence_nr = input_packet.message->get_header().sequence_nr();
            session.push_input_message(std::move(input_packet.message), stream_id, sequence_nr);
            while (session.pop_input_message(stream_id, input_packet.message))
            {
                process_input_message(*client, input_packet);
            }

            if (is_reliable_stream(stream_id))
            {
                dds::xrce::MessageHeader acknack_header;
                acknack_header.session_id(header.session_id());
                acknack_header.stream_id(dds::xrce::STREAMID_NONE);
                acknack_header.sequence_nr(0x00);
                acknack_header.client_key(header.client_key());

                dds::xrce::ACKNACK_Payload acknack_payload;
                client->session().fill_acknack(stream_id, acknack_payload);
                acknack_payload.stream_id(header.stream_id());

                dds::xrce::SubmessageHeader acknack_subheader;
                acknack_subheader.submessage_id(dds::xrce::ACKNACK);
                acknack_subheader.flags(dds::xrce::FLAG_LITTLE_ENDIANNESS);
                acknack_subheader.submessage_length(uint16_t(acknack_payload.getCdrSerializedSize()));

                const size_t message_size = acknack_header.getCdrSerializedSize() +
                                            acknack_subheader.getCdrSerializedSize() +
                                            acknack_payload.getCdrSerializedSize();

                OutputPacket<EndPoint> output_packet;
                output_packet.destination = input_packet.source;
                output_packet.message.reset(new OutputMessage(acknack_header, message_size));
                output_packet.message->append_submessage(dds::xrce::ACKNACK, acknack_payload);

                server_.push_output_packet(std::move(output_packet));
            }
        }
        else
        {
            if (input_packet.message->prepare_next_submessage())
            {
                switch (input_packet.message->get_subheader().submessage_id())
                {
                    case dds::xrce::DELETE_ID:
                    {
                        dds::xrce::DELETE_Payload delete_payload;
                        if (input_packet.message->get_payload(delete_payload)
                            && ((delete_payload.object_id().at(1) & 0x0F) == dds::xrce::OBJK_CLIENT))
                        {
                            dds::xrce::STATUS_Payload status_payload;
                            status_payload.related_request().request_id(delete_payload.request_id());
                            status_payload.related_request().object_id(delete_payload.object_id());
                            status_payload.result().status() = dds::xrce::STATUS_OK;

                            const size_t message_size =
                                input_packet.message->get_header().getCdrSerializedSize()
                                + input_packet.message->get_subheader().getCdrSerializedSize()
                                + status_payload.getCdrSerializedSize();

                            OutputPacket<EndPoint> output_packet;
                            output_packet.destination = input_packet.source;
                            output_packet.message.reset(new OutputMessage(input_packet.message->get_header(), message_size));
                            output_packet.message->append_submessage(dds::xrce::STATUS, status_payload);

                            server_.push_output_packet(std::move(output_packet));
                        }
                        break;
                    }
                    // Handle in session ping from an unknown client
                    case dds::xrce::GET_INFO:
                    {
                        OutputPacket<EndPoint> output_packet;
                        process_get_info_packet(std::move(input_packet), output_packet);
                        server_.push_output_packet(std::move(output_packet));
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
        }
    }
}

template<typename EndPoint>
void Processor<EndPoint>::process_input_message(
        ProxyClient& client,
        InputPacket<EndPoint>& input_packet)
{
    while (input_packet.message->prepare_next_submessage() && process_submessage(client, input_packet))
    {
    }
}

template<typename EndPoint>
bool Processor<EndPoint>::process_submessage(
        ProxyClient& client,
        InputPacket<EndPoint>& input_packet)
{
    bool rv;
    dds::xrce::SubmessageId submessage_id = input_packet.message->get_subheader().submessage_id();
    switch (submessage_id)
    {
        case dds::xrce::CREATE_CLIENT:
            rv = process_create_client_submessage(input_packet);
            break;
        case dds::xrce::CREATE:
            rv = process_create_submessage(client, input_packet);
            break;
        case dds::xrce::GET_INFO:
            rv = process_get_info_submessage(client, input_packet);
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
        default:
            rv = false;
            break;
    }
    return rv;
}

template<typename EndPoint>
bool Processor<EndPoint>::process_create_client_submessage(
        InputPacket<EndPoint>& input_packet)
{
    bool rv = true;
    dds::xrce::CREATE_CLIENT_Payload client_payload;
    if (((input_packet.message->get_header().session_id() == dds::xrce::SESSIONID_NONE_WITH_CLIENT_KEY) ||
         (input_packet.message->get_header().session_id() == dds::xrce::SESSIONID_NONE_WITHOUT_CLIENT_KEY)) &&
          input_packet.message->get_payload(client_payload))
    {
        dds::xrce::ClientKey client_key;
        uint32_t raw_client_key;
        if (server_.get_client_key(input_packet.source, raw_client_key) &&
            (client_payload.client_representation().client_key() != conversion::raw_to_clientkey(raw_client_key)))
        {
            client_key = conversion::raw_to_clientkey(raw_client_key);
            dds::xrce::StatusValue delete_status = root_.delete_client(client_key).status();
            if ((dds::xrce::STATUS_OK == delete_status) ||
                (dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE == delete_status))
            {
                server_.destroy_session(input_packet.source);
            }
            else
            {
                rv = false;
            }
        }

        if (rv)
        {
            dds::xrce::MessageHeader status_header = input_packet.message->get_header();
            status_header.session_id(client_payload.client_representation().session_id());

            dds::xrce::AGENT_Representation agent_representation;
            dds::xrce::ResultStatus result = root_.create_client(
                        client_payload.client_representation(),
                        agent_representation,
                        middleware_kind_);

            if (dds::xrce::STATUS_OK == result.status())
            {
                server_.establish_session(input_packet.source,
                                          conversion::clientkey_to_raw(client_payload.client_representation().client_key()),
                                          client_payload.client_representation().session_id());
            }

            dds::xrce::STATUS_AGENT_Payload status_agent;
            status_agent.result(result);
            status_agent.agent_info(agent_representation);

            dds::xrce::SubmessageHeader status_subheader;
            status_subheader.submessage_id(dds::xrce::STATUS_AGENT);
            status_subheader.flags(dds::xrce::FLAG_LITTLE_ENDIANNESS);
            status_subheader.submessage_length(uint16_t(status_agent.getCdrSerializedSize()));

            const size_t message_size = status_header.getCdrSerializedSize() +
                                        status_subheader.getCdrSerializedSize() +
                                        status_agent.getCdrSerializedSize();

            OutputPacket<EndPoint> output_packet;
            output_packet.destination = input_packet.source;
            output_packet.message = std::shared_ptr<OutputMessage>(new OutputMessage(status_header, message_size));
            output_packet.message->append_submessage(dds::xrce::STATUS_AGENT, status_agent);

            server_.push_output_packet(std::move(output_packet));
        }
    }
    else
    {
        rv = false;
    }

    return rv;
}

template<typename EndPoint>
bool Processor<EndPoint>::process_create_submessage(
        ProxyClient& client,
        InputPacket<EndPoint>& input_packet)
{
    bool rv = true;
    dds::xrce::CreationMode creation_mode;
    dds::xrce::StreamId stream_kind = is_reliable_stream(input_packet.message->get_header().stream_id()) ?
            dds::xrce::STREAMID_BUILTIN_RELIABLE : dds::xrce::STREAMID_BUILTIN_BEST_EFFORTS;

    creation_mode.reuse(0 < (input_packet.message->get_subheader().flags() & dds::xrce::FLAG_REUSE));
    creation_mode.replace(0 < (input_packet.message->get_subheader().flags() & dds::xrce::FLAG_REPLACE));

    dds::xrce::CREATE_Payload create_payload;
    if (input_packet.message->get_payload(create_payload))
    {
        dds::xrce::Endianness endianness = static_cast<dds::xrce::Endianness>(input_packet.message->get_subheader().flags() & 0x01);
        create_payload.object_representation().endianness(endianness);

        dds::xrce::STATUS_Payload status_payload;
        status_payload.related_request().request_id(create_payload.request_id());
        status_payload.related_request().object_id(create_payload.object_id());
        status_payload.result(client.create_object(creation_mode,
                                                   create_payload.object_id(),
                                                   create_payload.object_representation()));

        client.session().push_output_submessage(
            stream_kind,
            dds::xrce::STATUS,
            status_payload,
            std::chrono::milliseconds(0));

        OutputPacket<EndPoint> output_packet;
        output_packet.destination = input_packet.source;
        while (client.session().get_next_output_message(stream_kind, output_packet.message))
        {
            server_.push_output_packet(std::move(output_packet));
        }
    }
    return rv;
}

template<typename EndPoint>
bool Processor<EndPoint>::process_delete_submessage(
        ProxyClient& client,
        InputPacket<EndPoint>& input_packet)
{
    bool rv = true;
    dds::xrce::DELETE_Payload delete_payload;
    if (input_packet.message->get_payload(delete_payload))
    {
        dds::xrce::STATUS_Payload status_payload;
        status_payload.related_request().request_id(delete_payload.request_id());
        status_payload.related_request().object_id(delete_payload.object_id());

        OutputPacket<EndPoint> output_packet;
        output_packet.destination = input_packet.source;

        if ((delete_payload.object_id().at(1) & 0x0F) == dds::xrce::OBJK_CLIENT)
        {
            dds::xrce::ClientKey client_key = client.get_client_key();
            status_payload.result(root_.delete_client(client_key));
            if (dds::xrce::STATUS_OK == status_payload.result().status())
            {
                server_.destroy_session(input_packet.source);
            }

            client.session().push_output_submessage(
                dds::xrce::STREAMID_NONE,
                dds::xrce::STATUS,
                status_payload,
                std::chrono::milliseconds(0));

            while (client.session().get_next_output_message(dds::xrce::STREAMID_NONE, output_packet.message))
            {
                server_.push_output_packet(std::move(output_packet));
            }
        }
        else
        {
            dds::xrce::StreamId stream_kind = is_reliable_stream(input_packet.message->get_header().stream_id()) ?
                    dds::xrce::STREAMID_BUILTIN_RELIABLE : dds::xrce::STREAMID_BUILTIN_BEST_EFFORTS;

            status_payload.result(client.delete_object(delete_payload.object_id()));

            client.session().push_output_submessage(
                stream_kind,
                dds::xrce::STATUS,
                status_payload,
                std::chrono::milliseconds(0));

            while (client.session().get_next_output_message(stream_kind, output_packet.message))
            {
                server_.push_output_packet(std::move(output_packet));
            }
        }
    }
    else
    {
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("deserialization error processing DELETE submessage"),
            UXR_CLIENT_KEY_PATTERN,
            conversion::clientkey_to_raw(client.get_client_key()));
        rv = false;
    }
    return rv;
}

template<typename EndPoint>
bool Processor<EndPoint>::process_write_data_submessage(
        ProxyClient& client,
        InputPacket<EndPoint>& input_packet)
{
    bool deserialized = false, written = false;
    uint8_t flags = input_packet.message->get_subheader().flags() & 0x0E;
    size_t submessage_length = input_packet.message->get_subheader().submessage_length();

#ifdef UAGENT_TWEAK_XRCE_WRITE_LIMIT
    if (submessage_length == 0)
    {
        submessage_length =
            input_packet.message->get_len()
            - input_packet.message->get_header().getCdrSerializedSize(0)
            - input_packet.message->get_subheader().getCdrSerializedSize(0);
    }
#endif

    switch (flags)
    {
        case dds::xrce::FORMAT_DATA_FLAG:
        {
            dds::xrce::WRITE_DATA_Payload_Data data_payload;
            data_payload.data().resize(submessage_length - data_payload.BaseObjectRequest::getCdrSerializedSize(0));
            if (input_packet.message->get_payload(data_payload))
            {
                const dds::xrce::ObjectId& object_id = data_payload.object_id();
                switch (object_id[1] & 0x0F)
                {
                    case dds::xrce::OBJK_DATAWRITER:
                    {
                        std::shared_ptr<DataWriter> data_writer =
                                std::dynamic_pointer_cast<DataWriter>(client.get_object(object_id));
                        if (nullptr != data_writer)
                        {
                            written = data_writer->write(data_payload);
                        }
                        break;
                    }
                    case dds::xrce::OBJK_REQUESTER:
                    {
                        std::shared_ptr<Requester> requester =
                                std::dynamic_pointer_cast<Requester>(client.get_object(object_id));
                        if (nullptr != requester)
                        {
                            written = requester->write(data_payload, data_payload.request_id());
                        }
                        break;
                    }
                    case dds::xrce::OBJK_REPLIER:
                    {
                        std::shared_ptr<Replier> replier =
                                std::dynamic_pointer_cast<Replier>(client.get_object(object_id));
                        if (nullptr != replier)
                        {
                            written = replier->write(data_payload);
                        }
                        break;
                    }
                    default:
                        UXR_AGENT_LOG_ERROR(
                            UXR_DECORATE_RED("invalid ObjectId"),
                            UXR_CREATE_OBJECT_PATTERN,
                            conversion::objectid_to_raw(object_id));
                        break;
                }
                deserialized = true;
            }
            else
            {
                UXR_AGENT_LOG_ERROR(
                    UXR_DECORATE_RED("deserialization error processing WRITE_DATA submessage"),
                    UXR_CLIENT_KEY_PATTERN,
                    conversion::clientkey_to_raw(client.get_client_key()));
            }
            break;
        }
        default:
            break;
    }

    return deserialized && written;
}

template<typename EndPoint>
bool Processor<EndPoint>::process_read_data_submessage(
        ProxyClient& client,
        InputPacket<EndPoint>& input_packet)
{
    bool rv = true;
    dds::xrce::READ_DATA_Payload read_payload;
    if (input_packet.message->get_payload(read_payload))
    {
        const dds::xrce::ObjectId& object_id = read_payload.object_id();
        std::shared_ptr<XRCEObject> reader_object;

        switch (object_id[1] & 0x0F)
        {
            case dds::xrce::OBJK_DATAREADER:
                reader_object = std::dynamic_pointer_cast<DataReader>(client.get_object(read_payload.object_id()));
                break;
            case dds::xrce::OBJK_REQUESTER:
                reader_object = std::dynamic_pointer_cast<Requester>(client.get_object(read_payload.object_id()));
                break;
            case dds::xrce::OBJK_REPLIER:
                reader_object = std::dynamic_pointer_cast<Replier>(client.get_object(read_payload.object_id()));
                break;
            default:
                break;
        }

        dds::xrce::StatusValue status = (nullptr != reader_object)
                ? dds::xrce::STATUS_OK
                : dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE;

        if (dds::xrce::STATUS_OK == status)
        {
            WriteFnArgs write_args;
            write_args.client_key = client.get_client_key();
            write_args.stream_id = read_payload.read_specification().preferred_stream_id();
            write_args.object_id = read_payload.object_id();
            write_args.request_id = read_payload.request_id();

            using namespace std::placeholders;
            Reader<bool>::WriteFn write_fn = std::bind(&Processor::read_data_callback, this, _1, _2, _3);
            bool reading = false;

            switch (object_id[1] & 0x0F)
            {
                case dds::xrce::OBJK_DATAREADER:
                    reading = std::dynamic_pointer_cast<DataReader>(reader_object)->read(read_payload, write_fn, write_args);
                    break;
                case dds::xrce::OBJK_REQUESTER:
                    reading = std::dynamic_pointer_cast<Requester>(reader_object)->read(read_payload, write_fn, write_args);
                    break;
                case dds::xrce::OBJK_REPLIER:
                    reading = std::dynamic_pointer_cast<Replier>(reader_object)->read(read_payload, write_fn, write_args);
                    break;
                default:
                    break;
            }

            if (!reading)
            {
                status = dds::xrce::STATUS_ERR_RESOURCES;
            }
        }

        if (dds::xrce::STATUS_OK != status)
        {
            dds::xrce::STATUS_Payload status_payload;
            status_payload.related_request().request_id(read_payload.request_id());
            status_payload.related_request().object_id(read_payload.object_id());
            status_payload.result().implementation_status(0x00);
            status_payload.result().status(status);

            client.session().push_output_submessage(
                dds::xrce::STREAMID_BUILTIN_RELIABLE,
                dds::xrce::STATUS,
                status_payload,
                std::chrono::milliseconds(0));

            OutputPacket<EndPoint> output_packet;
            output_packet.destination = input_packet.source;
            while (client.session().get_next_output_message(dds::xrce::STREAMID_BUILTIN_RELIABLE, output_packet.message))
            {
                server_.push_output_packet(std::move(output_packet));
            }
        }
    }
    else
    {
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("deserialization error processing WRITE_DATA submessage"),
            UXR_CLIENT_KEY_PATTERN,
            conversion::clientkey_to_raw(client.get_client_key()));
        rv = false;
    }
    return rv;
}

template<typename EndPoint>
bool Processor<EndPoint>::process_acknack_submessage(
        ProxyClient& client,
        InputPacket<EndPoint>& input_packet)
{
    bool rv = true;
    dds::xrce::ACKNACK_Payload acknack_payload;
    if (input_packet.message->get_payload(acknack_payload))
    {
        uint16_t first_message = acknack_payload.first_unacked_seq_num();
        std::array<uint8_t, 2> nack_bitmap = acknack_payload.nack_bitmap();
        uint8_t stream_id = acknack_payload.stream_id();
        for (uint16_t i = 0; i < 8; ++i)
        {
            OutputPacket<EndPoint> output_packet;
            output_packet.destination = input_packet.source;
            uint8_t mask = uint8_t(0x01 << i);
            if ((nack_bitmap.at(1) & mask) == mask)
            {
                if (client.session().get_output_message(stream_id, first_message + i, output_packet.message))
                {
                    server_.push_output_packet(std::move(output_packet));
                }
            }
            if ((nack_bitmap.at(0) & mask) == mask)
            {
                if (client.session().get_output_message(stream_id, first_message + i + 8, output_packet.message))
                {
                    server_.push_output_packet(std::move(output_packet));
                }
            }
        }

        client.session().update_from_acknack(stream_id, first_message);
    }
    else
    {
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("deserialization error processing ACKNACK submessage"),
            UXR_CLIENT_KEY_PATTERN,
            conversion::clientkey_to_raw(client.get_client_key()));
        rv = false;
    }
    return rv;
}

template<typename EndPoint>
bool Processor<EndPoint>::process_heartbeat_submessage(
        ProxyClient& client,
        InputPacket<EndPoint>& input_packet)
{
    bool rv = true;
    dds::xrce::HEARTBEAT_Payload heartbeat_payload;
    if (input_packet.message->get_payload(heartbeat_payload))
    {
        uint8_t stream_id = heartbeat_payload.stream_id();
        client.session().update_from_heartbeat(stream_id,
                                               heartbeat_payload.first_unacked_seq_nr(),
                                               heartbeat_payload.last_unacked_seq_nr());

        dds::xrce::ACKNACK_Payload acknack_payload;
        client.session().fill_acknack(stream_id, acknack_payload);
        acknack_payload.stream_id(stream_id);

        client.session().push_output_submessage(
            dds::xrce::STREAMID_NONE,
            dds::xrce::ACKNACK,
            acknack_payload,
            std::chrono::milliseconds(0));

        OutputPacket<EndPoint> output_packet;
        output_packet.destination = input_packet.source;
        if (client.session().get_next_output_message(dds::xrce::STREAMID_NONE, output_packet.message))
        {
            server_.push_output_packet(std::move(output_packet));
        }
    }
    else
    {
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("deserialization error processing HEARTBEAT submessage"),
            UXR_CLIENT_KEY_PATTERN,
            conversion::clientkey_to_raw(client.get_client_key()));
        rv = false;
    }
    return rv;
}

template<typename EndPoint>
bool Processor<EndPoint>::process_reset_submessage(
        ProxyClient& client,
        InputPacket<EndPoint>& /*input_packet*/)
{
    client.session().reset();
    return true;
}

template<typename EndPoint>
bool Processor<EndPoint>::process_fragment_submessage(
        ProxyClient& client,
        InputPacket<EndPoint>& input_packet)
{
    dds::xrce::StreamId stream_id = input_packet.message->get_header().stream_id();
    client.session().push_input_fragment(stream_id, input_packet.message);
    InputPacket<EndPoint> fragment_packet;
    if (client.session().pop_input_fragment_message(stream_id, fragment_packet.message))
    {
        fragment_packet.source = input_packet.source;
        process_input_message(client, fragment_packet);
    }
    return true;
}

template<typename EndPoint>
bool Processor<EndPoint>::process_timestamp_submessage(
        ProxyClient& client,
        InputPacket<EndPoint>& input_packet)
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

        client.session().push_output_submessage(
            dds::xrce::STREAMID_NONE,
            dds::xrce::TIMESTAMP_REPLY,
            timestamp_reply,
            std::chrono::milliseconds(0));

        OutputPacket<EndPoint> output_packet;
        output_packet.destination = input_packet.source;
        if (client.session().get_next_output_message(dds::xrce::STREAMID_NONE, output_packet.message))
        {
            server_.push_output_packet(std::move(output_packet));
        }
    }
    else
    {
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("deserialization error processing TIMESTAMP submessage"),
            UXR_CLIENT_KEY_PATTERN,
            conversion::clientkey_to_raw(client.get_client_key()));
        rv = false;
    }
    return rv;
}

template<typename EndPoint>
bool Processor<EndPoint>::process_get_info_submessage(
        ProxyClient& client,
        InputPacket<EndPoint>& input_packet)
{
    bool rv = false;

    dds::xrce::GET_INFO_Payload get_info_payload;
    input_packet.message->get_payload(get_info_payload);

    dds::xrce::ObjectInfo object_info;
    dds::xrce::ResultStatus result_status = root_.get_info(object_info);
    if (dds::xrce::STATUS_OK == result_status.status())
    {
        result_status.implementation_status(1);

        dds::xrce::AGENT_ActivityInfo agent_info;
        agent_info.availability(1);

        dds::xrce::ActivityInfoVariant info_variant;
        info_variant.agent(agent_info);
        object_info.activity(info_variant);

        dds::xrce::INFO_Payload info_payload;
        info_payload.related_request().request_id(get_info_payload.request_id());
        info_payload.related_request().object_id(get_info_payload.object_id());
        info_payload.result(result_status);
        info_payload.object_info(object_info);

        dds::xrce::SubmessageHeader info_subheader;
        info_subheader.submessage_id(dds::xrce::INFO);
        info_subheader.flags(dds::xrce::FLAG_LITTLE_ENDIANNESS);
        info_subheader.submessage_length(uint16_t(info_payload.getCdrSerializedSize()));

        OutputPacket<EndPoint> output_packet;
        output_packet.destination = input_packet.source;

        dds::xrce::MessageHeader header;
        header.session_id(client.get_session_id());
        header.client_key(client.get_client_key());

        const size_t message_size =
            header.getCdrSerializedSize() +
            info_subheader.getCdrSerializedSize() +
            info_payload.getCdrSerializedSize();

        output_packet.message = OutputMessagePtr(new OutputMessage(header, message_size));
        rv = output_packet.message->append_submessage(dds::xrce::INFO, info_payload);

        server_.push_output_packet(std::move(output_packet));
    }

    return rv;
}

template<typename EndPoint>
bool Processor<EndPoint>::read_data_callback(
        const WriteFnArgs& cb_args,
        const std::vector<uint8_t>& buffer,
        std::chrono::milliseconds timeout)
{
    bool rv = false;

    dds::xrce::DATA_Payload_Data data_payload;
    data_payload.request_id(cb_args.request_id);
    data_payload.object_id(cb_args.object_id);
    data_payload.data().serialized_data(buffer);

    OutputPacket<EndPoint> output_packet;
    if (server_.get_endpoint(conversion::clientkey_to_raw(cb_args.client_key), output_packet.destination))
    {
        rv = cb_args.client->session().push_output_submessage(cb_args.stream_id, dds::xrce::DATA, data_payload, timeout);

        while (cb_args.client->session().get_next_output_message(cb_args.stream_id, output_packet.message))
        {
            server_.push_output_packet(std::move(output_packet));
        }
    }
    else
    {
        std::this_thread::sleep_for(timeout);
    }
    return rv;
}

template<typename EndPoint>
bool Processor<EndPoint>::process_get_info_packet(
        InputPacket<EndPoint>&& input_packet,
        OutputPacket<EndPoint>& output_packet) const
{
    bool rv = false;

    dds::xrce::GET_INFO_Payload get_info_payload;
    input_packet.message->get_payload(get_info_payload);

    dds::xrce::ObjectInfo object_info;
    dds::xrce::ResultStatus result_status = root_.get_info(object_info);
    if (dds::xrce::STATUS_OK == result_status.status())
    {
        uint32_t raw_client_key;
        if (server_.get_client_key(input_packet.source, raw_client_key))
        {
            result_status.implementation_status(1);
        }
        else
        {
            result_status.implementation_status(0);
        }

        dds::xrce::AGENT_ActivityInfo agent_info;
        agent_info.availability(1);

        dds::xrce::ActivityInfoVariant info_variant;
        info_variant.agent(agent_info);
        object_info.activity(info_variant);

        dds::xrce::INFO_Payload info_payload;
        info_payload.related_request().request_id(get_info_payload.request_id());
        info_payload.related_request().object_id(get_info_payload.object_id());
        info_payload.result(result_status);
        info_payload.object_info(object_info);

        dds::xrce::SubmessageHeader info_subheader;
        info_subheader.submessage_id(dds::xrce::INFO);
        info_subheader.flags(dds::xrce::FLAG_LITTLE_ENDIANNESS);
        info_subheader.submessage_length(uint16_t(info_payload.getCdrSerializedSize()));

        const size_t message_size = input_packet.message->get_header().getCdrSerializedSize() +
                                    info_subheader.getCdrSerializedSize() +
                                    info_payload.getCdrSerializedSize();

        output_packet.destination = input_packet.source;
        output_packet.message = OutputMessagePtr(new OutputMessage(input_packet.message->get_header(),
                                                                    message_size));
        rv = output_packet.message->append_submessage(dds::xrce::INFO, info_payload);
    }

    return rv;
}

template<typename EndPoint>
bool Processor<EndPoint>::process_get_info_packet(
        InputPacket<IPv4EndPoint>&& input_packet,
        std::vector<dds::xrce::TransportAddress>& address,
        OutputPacket<IPv4EndPoint>& output_packet) const
{
    bool rv = false;

    if (input_packet.message->prepare_next_submessage())
    {
        if (input_packet.message->get_subheader().submessage_id() == dds::xrce::GET_INFO)
        {
            dds::xrce::GET_INFO_Payload get_info_payload;
            input_packet.message->get_payload(get_info_payload);

            dds::xrce::ObjectInfo object_info;
            dds::xrce::ResultStatus result_status = root_.get_info(object_info);
            if (dds::xrce::STATUS_OK == result_status.status())
            {
                dds::xrce::AGENT_ActivityInfo agent_info;
                for (auto &a : address)
                {
                    agent_info.address_seq().push_back(a);
                }
                agent_info.availability(1);

                dds::xrce::ActivityInfoVariant info_variant;
                info_variant.agent(agent_info);
                object_info.activity(info_variant);

                dds::xrce::INFO_Payload info_payload;
                info_payload.related_request().request_id(get_info_payload.request_id());
                info_payload.related_request().object_id(get_info_payload.object_id());
                info_payload.result(result_status);
                info_payload.object_info(object_info);

                dds::xrce::SubmessageHeader info_subheader;
                info_subheader.submessage_id(dds::xrce::INFO);
                info_subheader.flags(dds::xrce::FLAG_LITTLE_ENDIANNESS);
                info_subheader.submessage_length(uint16_t(info_payload.getCdrSerializedSize()));

                const size_t message_size = input_packet.message->get_header().getCdrSerializedSize() +
                                            info_subheader.getCdrSerializedSize() +
                                            info_payload.getCdrSerializedSize();

                output_packet.destination = input_packet.source;
                output_packet.message = OutputMessagePtr(new OutputMessage(input_packet.message->get_header(),
                                                                           message_size));
                rv = output_packet.message->append_submessage(dds::xrce::INFO, info_payload);
            }
        }
    }

    return rv;
}

template<typename EndPoint>
void Processor<EndPoint>::check_heartbeats()
{
    dds::xrce::MessageHeader header;
    header.stream_id(dds::xrce::STREAMID_NONE);
    header.sequence_nr(0x00);

    dds::xrce::HEARTBEAT_Payload heartbeat;

    dds::xrce::SubmessageHeader subheader;
    subheader.submessage_id(dds::xrce::HEARTBEAT);
    subheader.flags(dds::xrce::FLAG_LITTLE_ENDIANNESS);
    subheader.submessage_length(uint16_t(heartbeat.getCdrSerializedSize()));

    const size_t message_size =
            header.getCdrSerializedSize() +
            subheader.getCdrSerializedSize() +
            heartbeat.getCdrSerializedSize();

    OutputPacket<EndPoint> output_packet;

    std::shared_ptr<ProxyClient> client;
    while (root_.get_next_client(client))
    {
        ProxyClient::State state = client->get_state();
        uint32_t raw_key = conversion::clientkey_to_raw(client->get_client_key());

        if (server_.get_endpoint(raw_key, output_packet.destination) &&
             ProxyClient::State::alive == state)
        {
            header.session_id(client->get_session_id());
            header.client_key(client->get_client_key());

            for (auto stream : client->session().get_output_streams())
            {
                if (client->session().fill_heartbeat(stream, heartbeat))
                {
                    output_packet.message = OutputMessagePtr(new OutputMessage(header, message_size));
                    output_packet.message->append_submessage(dds::xrce::HEARTBEAT, heartbeat);

                    server_.push_output_packet(std::move(output_packet));
                }
            }
        }
        else if (client->has_hard_liveliness_check() && ProxyClient::State::dead == state)
        {
            client->get_hard_liveliness_check_tries()++;
            if (client->get_hard_liveliness_check_tries() == 3)
            {
                client->update_state(ProxyClient::State::to_remove);
            }

            dds::xrce::GET_INFO_Payload get_info_payload = {};

            const size_t get_info_size =
                header.getCdrSerializedSize() +
                subheader.getCdrSerializedSize() +
                get_info_payload.getCdrSerializedSize();

            header.session_id(client->get_session_id());
            header.client_key(client->get_client_key());

            get_info_payload.request_id({0,0});
            get_info_payload.object_id(dds::xrce::OBJECTID_CLIENT);

            output_packet.message = OutputMessagePtr(new OutputMessage(header, get_info_size));
            output_packet.message->append_submessage(dds::xrce::GET_INFO, get_info_payload);

            server_.push_output_packet(std::move(output_packet));
        }
        else if (client->has_hard_liveliness_check() &&ProxyClient::State::to_remove == state)
        {
            if (dds::xrce::STATUS_OK == root_.delete_client(client->get_client_key()).status())
            {
                server_.destroy_session(conversion::clientkey_to_raw(client->get_client_key()));

                UXR_AGENT_LOG_INFO(
                    UXR_DECORATE_YELLOW("Session destroyed due to liveliness timeout"),
                    "client_key: 0x{:08X}, address: {}",
                    raw_key,
                    output_packet.destination);
            }
        }
    }
}

template class Processor<IPv4EndPoint>;
template class Processor<IPv6EndPoint>;
template class Processor<CanEndPoint>;
template class Processor<SerialEndPoint>;
template class Processor<MultiSerialEndPoint>;
template class Processor<CustomEndPoint>;

} // namespace uxr
} // namespace eprosima
