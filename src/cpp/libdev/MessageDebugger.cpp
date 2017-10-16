#include "MessageDebugger.h"

#include <DDSXRCETypes.h>
#include <MessageHeader.h>
#include <Payloads.h>
#include <SubMessageHeader.h>

#include <iomanip>

namespace eprosima {
namespace micrortps {
namespace debug {

#define SEPARATOR " | "

std::ostream& operator<<(std::ostream& stream, const std::vector<unsigned char>& values)
{
    StreamScopedFlags flag_backup{stream};
    stream << std::setfill('0') << std::setw(2) << static_cast<uint8_t>(values[0]);
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const MessageHeader& header)
{
    stream << std::showbase << std::internal << std::setfill('0') << std::hex;
    stream << "<Header>" << std::endl;
    stream << "  - client_key: " << std::setw(10) << header.client_key() << std::endl;
    stream << "  - session_id: " << std::setw(4) << +header.session_id() << std::endl;
    stream << "  - stream_id: " << std::setw(4) << +header.stream_id() << std::endl;
    stream << "  - sequence_nr: " << std::setw(6) << header.sequence_nr();
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const SubmessageHeader& submessage_header)
{
    stream << std::showbase << std::internal << std::setfill('0') << std::hex;
    switch(submessage_header.submessage_id())
    {
        case CREATE:
            stream << "<Submessage> [CREATE]" << std::endl;
            break;
        case DELETE:
            stream << "<Submessage> [DELETE]" << std::endl;
            break;
        case WRITE_DATA:
            stream << "<Submessage> [WRITE_DATA]" << std::endl;
            break;
        case READ_DATA:
            stream << "<Submessage> [READ_DATA]" << std::endl;
            break;
        case DATA:
            stream << "<Submessage> [DATA]" << std::endl;
            break;
        case STATUS:
            stream << "<Submessage> [STATUS]" << std::endl;
            break;
    }
    stream << "  <Submessage header> " << std::endl;
    stream << "  - id: " << std::setw(4) << +submessage_header.submessage_id() << std::endl;
    stream << "  - flags: " << std::setw(4) << +submessage_header.flags() << std::endl;
    stream << "  - length: " << std::setw(6) << submessage_header.submessage_length();
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const RESOURCE_STATUS_Payload& status)
{
    stream << std::showbase << std::internal << std::setfill('0') << std::hex;
    stream << "  <Payload>" << std::endl;
    stream << "  - result : " << status.result() << std::endl;
    stream << "  - object_id: " << status.object_id();
    return stream;
}

std::ostream& short_print(std::ostream& stream, const RESOURCE_STATUS_Payload& status, const STREAM_COLOR color)
{
    ColorStream cs(stream, color);
    stream << "[Status" << SEPARATOR;
    stream << "id: " << status.object_id() << SEPARATOR;
    short_print(stream, status.result()) << "]";
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const ResultStatus& status)
{
    stream << "  <ResultStatus>" << std::endl;
    stream << "  - request_id : " << status.request_id() << std::endl;
    stream << "  - status: " << std::setw(4) << +status.status() << std::endl;
    stream << "  - implementation_status: " << std::setw(4) << +status.implementation_status();
    return stream;
}

std::ostream& short_print(std::ostream& stream, const ResultStatus& status, const STREAM_COLOR color)
{
    ColorStream cs(stream, color);

    stream << "#" << std::setfill('0') << std::setw(8) << +status.request_id()[0] << SEPARATOR;

    switch(status.implementation_status())
    {
        case STATUS_OK:
            stream << "OK";
            break;
        case STATUS_OK_MATCHED:
            stream << "OK_MATCHED";
            break;
        case STATUS_ERR_DDS_ERROR:
            stream << "ERR_DDS_ERROR";
            break;
        case STATUS_ERR_MISMATCH:
            stream << "ERR_MISMATCH";
            break;
        case STATUS_ERR_ALREADY_EXISTS:
            stream << "ERR_ALREADY_EXISTS";
            break;
        case STATUS_ERR_DENIED:
            stream << "ERR_DENIED";
            break;
        case STATUS_ERR_UNKNOWN_REFERENCE:
            stream << "ERR_UNKNOWN_REFERENCE";
            break;
        case STATUS_ERR_INVALID_DATA:
            stream << "ERR_INVALID_DATA";
            break;
        case STATUS_ERR_INCOMPATIBLE:
            stream << "ERR_INCOMPATIBLE";
            break;
        case STATUS_ERR_RESOURCES:
            stream << "ERR_RESOURCES";
            break;
        default:
            stream << "UNKNOWN";
    }
    stream << SEPARATOR;
    switch(status.status())
    {
        case STATUS_LAST_OP_NONE:
            stream << "NONE";
            break;
        case STATUS_LAST_OP_CREATE:
            stream << "CREATE";
            break;
        case STATUS_LAST_OP_UPDATE:
            stream << "UPDATE";
            break;
        case STATUS_LAST_OP_DELETE:
            stream << "DELETE";
            break;
        case STATUS_LAST_OP_LOOKUP:
            stream << "LOOKUP";
            break;
        case STATUS_LAST_OP_READ:
            stream << "READ";
            break;
        case STATUS_LAST_OP_WRITE:
            stream << "WRITE";
            break;
        default:
            stream << "UNKNOWN";
    }
    return stream;
}

std::ostream& short_print(std::ostream& stream, const std::string& text, const STREAM_COLOR color)
{
    ColorStream cs(stream, color);
    StreamScopedFlags flags_backup{stream};
    stream << text;
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const CREATE_Payload& create_payload)
{
    StreamScopedFlags flags_backup{stream};
    stream << std::showbase << std::internal << std::setfill('0') << std::hex;
    stream << "  <Payload>" << std::endl;
    stream << "  - request_id : " << create_payload.request_id() << std::endl;
    stream << "  - object_id: " << create_payload.object_id() << std::endl;
    stream << "  - object_representation: " << create_payload.object_representation();
    return stream;
}

std::ostream& short_print(std::ostream& stream, const CREATE_Payload& create_payload, const STREAM_COLOR color)
{
    ColorStream cs(stream, color);
    stream << "[Create" << SEPARATOR;
    stream << "id: " << create_payload.object_id() << SEPARATOR;
    stream << "#" << std::setfill('0') << std::setw(8) << +create_payload.request_id()[0] << SEPARATOR;
    // switch(create_payload.object_representation().discriminator())
    // {
    //     case OBJK_PARTICIPANT:
    //         stream << "PARTICIPANT";
    //         break;
    //     case OBJK_PUBLISHER:
    //         stream << "PUBLISHER" << SEPARATOR;
    //         stream  << "id: " << create_payload.object_representation().publisher().participant_id() << SEPARATOR;
    //         stream << "topic: " << create_payload.object_representation().publisher().as_string();
    //     break;
    //     case OBJK_SUBSCRIBER:
    //         stream << "SUBSCRIBER" << SEPARATOR;
    //         stream  << "id: " << create_payload.object_representation().subscriber().participant_id() << SEPARATOR;
    //         stream << "topic: " << create_payload.object_representation().subscriber().as_string();
    //     break;
    //     case OBJK_CLIENT:
    //         stream << "OBJK_CLIENT" << SEPARATOR;
    //     break;
    //     default:
    //         stream << "UNKNOWN";
    //     break;
    // }
    // stream << "]";
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const OBJK_PUBLISHER_Representation& data)
{
    StreamScopedFlags flags_backup{stream};
    stream << std::showbase << std::internal << std::setfill('0') << std::hex;
    stream << "  <Publisher>" << std::endl;
    // stream << "  - string_size: " << std::setw(8) << data.as_string().size() << std::endl;
    // stream << "  - string: " << data.as_string() << std::endl;
    stream << "  - participan_id: " << data.participant_id();
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const OBJK_SUBSCRIBER_Representation& data)
{
    StreamScopedFlags flags_backup{stream};
    stream << std::showbase << std::internal << std::setfill('0') << std::hex;
    stream << "  <Subscriber>" << std::endl;
    // stream << "  - string_size: " << std::setw(8) << data.as_string().size() << std::endl;
    // stream << "  - string: " << data.as_string() << std::endl;
    stream << "  - participan_id: " << data.participant_id();
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const OBJK_DATAWRITER_Representation& data)
{
    StreamScopedFlags flags_backup{stream};
    stream << std::showbase << std::internal << std::setfill('0') << std::hex;
    stream << "  <Data writer>" << std::endl;
    // stream << "  - string_size: " << std::setw(8) << data.as_string().size() << std::endl;
    // stream << "  - string: " << data.as_string() << std::endl;
    // stream << "  - participan_id: " << data.participant_id() << std::endl;
    // stream << "  - publisher_id: " << data.publisher_id();
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const OBJK_DATA_READER_Representation& data)
{
    StreamScopedFlags flags_backup{stream};
    stream << std::showbase << std::internal << std::setfill('0') << std::hex;
    stream << "  <Data Reader>" << std::endl;
    // stream << "  - string_size: " << std::setw(8) << data.as_string().size() << std::endl;
    // stream << "  - string: " << data.as_string() << std::endl;
    stream << "  - participan_id: " << data.participant_id() << std::endl;
    stream << "  - subscriber_id: " << data.subscriber_id();
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const ObjectVariant& object_representation)
{
    StreamScopedFlags flags_backup{stream};
    stream << std::showbase << std::internal << std::setfill('0') << std::hex;
    // stream << "  - Kind " << std::setw(4) << +object_representation.discriminator() << std::endl;
    // switch(object_representation.discriminator())
    // {
    //     case OBJK_DATAWRITER:
    //         stream << object_representation.data_writer();
    //     break;

    //     case OBJK_DATAREADER:
    //         stream << object_representation.data_reader();
    //     break;

    //     case OBJK_SUBSCRIBER:
    //         stream << object_representation.subscriber();
    //     break;

    //     case OBJK_PUBLISHER:
    //         stream << object_representation.publisher();
    //     break;
    //     default:
    //         stream << "No string representation for this kind of object";
    //     break;
    // }

    return stream;
}

std::ostream& operator<<(std::ostream& stream, const DELETE_RESOURCE_Payload& delete_data)
{
    StreamScopedFlags flags_backup{stream};
    stream << std::showbase << std::internal << std::setfill('0') << std::hex;
    stream << "  <PayLoad>" << std::endl;
    stream << "  - request_id : " << delete_data.request_id() << std::endl;
    stream << "  - object_id: " << delete_data.object_id();
    return stream;
}

std::ostream& short_print(std::ostream& stream, const DELETE_RESOURCE_Payload& delete_data, const STREAM_COLOR color)
{
    ColorStream cs(stream, color);
    StreamScopedFlags flags_backup{stream};
    stream << "[Delete" << SEPARATOR;
    stream << "id: " << delete_data.object_id() << SEPARATOR;
    stream << "#" << std::setfill('0') << std::setw(8) << +delete_data.request_id()[0] << SEPARATOR;
    stream << "]";
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const WRITE_DATA_Payload& write_data)
{
    StreamScopedFlags flags_backup{stream};
    stream << std::showbase << std::internal << std::setfill('0') << std::hex;
    stream << "  <Payload>" << std::endl;
    stream << "  - request_id : " << write_data.request_id() << std::endl;
    stream << "  - object_id: " << write_data.object_id();
    // stream << "  - data: " << write_data.data_writer();
    return stream;
}

std::ostream& short_print(std::ostream& stream, const WRITE_DATA_Payload& write_data, const STREAM_COLOR color)
{
    ColorStream cs(stream, color);
    StreamScopedFlags flags_backup{stream};
    stream << "[Write data" << SEPARATOR;
    stream << "id: " << write_data.object_id() << SEPARATOR;
    stream << "#" << std::setfill('0') << std::setw(8) << +write_data.request_id()[0] << SEPARATOR;
    // switch(write_data.data_writer()._d())
    // {
    //     case READM_DATA:
    //     {
    //         stream << "DATA" << SEPARATOR << "data size: " <<
    //         write_data.data_writer().data().serialized_data().size(); break;
    //     }
    //     default:
    //     break;
    // }
    stream << "]";
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const READ_DATA_Payload& data)
{
    StreamScopedFlags flags_backup{stream};
    stream << std::showbase << std::internal << std::setfill('0') << std::hex;
    stream << "  <Payload>" << std::endl;
    stream << "  - request_id : " << data.request_id() << std::endl;
    stream << "  - object_id: " << data.object_id() << std::endl;
    if(data.read_specification().has_content_filter_expresion())
    {
        stream << "  - content filter size: " << data.read_specification().content_filter_expression().size() << std::endl;
        stream << "  - content filter: " << data.read_specification().content_filter_expression() << std::endl;
    }
    stream << "  - data format: " << data.read_specification().delivery_config()._d() << std::endl;
    stream << "  - max_elapsed_time: " << data.read_specification().delivery_config().delivery_control().max_elapsed_time() << std::endl;
    stream << "  - max_rate: " << data.read_specification().delivery_config().delivery_control().max_rate() << std::endl;
    stream << "  - max_samples: " << data.read_specification().delivery_config().delivery_control().max_samples() << std::endl; 
}

std::ostream& short_print(std::ostream& stream, const READ_DATA_Payload& read_data, const STREAM_COLOR color)
{
    ColorStream cs(stream, color);
    StreamScopedFlags flags_backup{stream};
    stream << "[Read data" << SEPARATOR;
    stream << "id: " << read_data.object_id() << SEPARATOR;
    stream << "#" << std::setfill('0') << std::setw(8) << +read_data.request_id()[0] << SEPARATOR;
    // stream << "max messages: " << read_data.max_messages();
    stream << "]";
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const DATA_Payload_Data& data)
{
    // TODO(borja)
    return stream;
}

std::ostream& short_print(std::ostream& stream, const DATA_Payload_Data& data, const STREAM_COLOR color)
{
    // TODO(borja)
    return stream;
}

} // namespace debug
} // namespace micrortps
} // namespace eprosima
