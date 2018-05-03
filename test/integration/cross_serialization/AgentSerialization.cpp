#include "AgentSerialization.hpp"
#include <agent/XRCETypes.h>
#include <agent/Serializer.h>

#define BUFFER_LENGTH 1024

std::vector<uint8_t> AgentSerialization::create_client_payload()
{
    std::vector<uint8_t> buffer(BUFFER_LENGTH, 0x00);
    eprosima::micrortps::Serializer serializer((char*)&buffer.front(), buffer.capacity());

    dds::xrce::CREATE_CLIENT_Payload payload;
    payload.request_id() = {0x01, 0x23};
    payload.object_id() = {0x45, 0x67};
    payload.client_representation().xrce_cookie() = {0x89, 0xAB, 0xCD, 0xEF};
    payload.client_representation().xrce_version() = {0x01, 0x23};
    payload.client_representation().xrce_vendor_id() = {0x45, 0x67};
    payload.client_representation().client_timestamp().seconds() = 0x89ABCDEF;
    payload.client_representation().client_timestamp().nanoseconds() = 0x01234567;
    payload.client_representation().client_key() = {0x89, 0xAB, 0xCD, 0xEF};
    payload.client_representation().session_id() = 0x01;
    serializer.serialize(payload);

    buffer.resize(serializer.get_serialized_size());

    return buffer;
}

std::vector<uint8_t> AgentSerialization::create_payload()
{
    std::vector<uint8_t> buffer(BUFFER_LENGTH, 0x00);
    eprosima::micrortps::Serializer serializer((char*)&buffer.front(), buffer.capacity());

    dds::xrce::CREATE_Payload payload;
    payload.request_id() = {0x01, 0x23};
    payload.object_id() = {0x45, 0x67};
    payload.object_representation()._d() = dds::xrce::OBJK_PARTICIPANT;
    payload.object_representation().participant().representation()._d() = dds::xrce::REPRESENTATION_BY_REFERENCE;
    payload.object_representation().participant().representation().object_reference() = "ABCDE";
    payload.object_representation().participant().domain_id() = (uint16_t)0x89AB;
    serializer.serialize(payload);

    buffer.resize(serializer.get_serialized_size());

    return buffer;
}

std::vector<uint8_t> AgentSerialization::get_info_payload()
{
    /*std::vector<uint8_t> buffer(BUFFER_LENGTH, 0x00);
    eprosima::micrortps::Serializer serializer((char*)&buffer.front(), buffer.capacity());

    dds::xrce::GET_INFO_Payload payload;
    payload.request_id() = {0x01, 0x23};
    payload.object_id() = {0x45, 0x67};
    payload.info_mask() = (dds::xrce::InfoMask)0x89ABCDEF;
    serializer.serialize(payload);

    buffer.resize(serializer.get_serialized_size());

    return buffer; */

    // Serializer not prepared for this payload.

    return std::vector<uint8_t>();
}

std::vector<uint8_t> AgentSerialization::delete_payload()
{
    std::vector<uint8_t> buffer(BUFFER_LENGTH, 0x00);
    eprosima::micrortps::Serializer serializer((char*)&buffer.front(), buffer.capacity());

    dds::xrce::DELETE_Payload payload;
    payload.request_id() = {0x01, 0x23};
    payload.object_id() = {0x45, 0x67};
    serializer.serialize(payload);

    buffer.resize(serializer.get_serialized_size());

    return buffer;
}

std::vector<uint8_t> AgentSerialization::status_agent_payload()
{
    //TODO
    return std::vector<uint8_t>();
}

std::vector<uint8_t> AgentSerialization::status_payload()
{
    std::vector<uint8_t> buffer(BUFFER_LENGTH, 0x00);
    eprosima::micrortps::Serializer serializer((char*)&buffer.front(), buffer.capacity());

    dds::xrce::STATUS_Payload payload;
    payload.related_request().request_id() = {0x01, 0x23};
    payload.related_request().object_id() = {0x45, 0x67};
    payload.result().implementation_status() = 0x89;
    payload.result().status() = (dds::xrce::StatusValue)0xAB;
    serializer.serialize(payload);

    buffer.resize(serializer.get_serialized_size());

    return buffer;
}

std::vector<uint8_t> AgentSerialization::info_payload()
{
    //TODO
    return std::vector<uint8_t>();
}

std::vector<uint8_t> AgentSerialization::read_data_payload()
{
    std::vector<uint8_t> buffer(BUFFER_LENGTH, 0x00);
    eprosima::micrortps::Serializer serializer((char*)&buffer.front(), buffer.capacity());

    dds::xrce::READ_DATA_Payload payload;
    payload.request_id() = {0x01, 0x23};
    payload.object_id() = {0x45, 0x67};
    payload.read_specification().data_format() = 0x89;
    serializer.serialize(payload);

    buffer.resize(serializer.get_serialized_size());

    return buffer;
}

std::vector<uint8_t> AgentSerialization::write_data_payload_data()
{
    std::vector<uint8_t> buffer(BUFFER_LENGTH, 0x00);
    eprosima::micrortps::Serializer serializer((char*)&buffer.front(), buffer.capacity());

    dds::xrce::WRITE_DATA_Payload_Data payload;
    payload.request_id() = {0x01, 0x23};
    payload.object_id() = {0x45, 0x67};
    payload.data().serialized_data() = {'B', 'Y', 'T', 'E', 'S'};
    serializer.serialize(payload);

    buffer.resize(serializer.get_serialized_size());

    return buffer;
}

std::vector<uint8_t> AgentSerialization::write_data_payload_sample()
{
    //TODO
    return std::vector<uint8_t>();
}

std::vector<uint8_t> AgentSerialization::write_data_payload_data_seq()
{
    //TODO
    return std::vector<uint8_t>();
}

std::vector<uint8_t> AgentSerialization::write_data_payload_sample_seq()
{
    //TODO
    return std::vector<uint8_t>();
}

std::vector<uint8_t> AgentSerialization::write_data_payload_packed_samples()
{
    //TODO
    return std::vector<uint8_t>();
}

std::vector<uint8_t> AgentSerialization::data_payload_data()
{
    std::vector<uint8_t> buffer(BUFFER_LENGTH, 0x00);
    eprosima::micrortps::Serializer serializer((char*)&buffer.front(), buffer.capacity());

    dds::xrce::DATA_Payload_Data payload;
    payload.request_id() = {0x01, 0x23};
    payload.object_id() = {0x45, 0x67};
    payload.data().serialized_data() = {'B', 'Y', 'T', 'E', 'S'};
    serializer.serialize(payload);

    buffer.resize(serializer.get_serialized_size());

    return buffer;
}

std::vector<uint8_t> AgentSerialization::data_payload_sample()
{
    //TODO
    return std::vector<uint8_t>();
}

std::vector<uint8_t> AgentSerialization::data_payload_data_seq()
{
    //TODO
    return std::vector<uint8_t>();
}

std::vector<uint8_t> AgentSerialization::data_payload_sample_seq()
{
    //TODO
    return std::vector<uint8_t>();
}

std::vector<uint8_t> AgentSerialization::data_payload_packed_samples()
{
    //TODO
    return std::vector<uint8_t>();
}

std::vector<uint8_t> AgentSerialization::acknack_payload()
{
    std::vector<uint8_t> buffer(BUFFER_LENGTH, 0x00);
    eprosima::micrortps::Serializer serializer((char*)&buffer.front(), buffer.capacity());

    dds::xrce::ACKNACK_Payload payload;
    payload.first_unacked_seq_num() = (uint16_t)0x0123;
    payload.nack_bitmap() = {0x45, 0x67};
    serializer.serialize(payload);

    buffer.resize(serializer.get_serialized_size());

    return buffer;
}

std::vector<uint8_t> AgentSerialization::heartbeat_payload()
{
    std::vector<uint8_t> buffer(BUFFER_LENGTH, 0x00);
    eprosima::micrortps::Serializer serializer((char*)&buffer.front(), buffer.capacity());

    dds::xrce::HEARTBEAT_Payload payload;
    payload.first_unacked_seq_nr() = (uint16_t)0x0123;
    payload.last_unacked_seq_nr() = (uint16_t)0x4567;
    serializer.serialize(payload);

    buffer.resize(serializer.get_serialized_size());

    return buffer;
}
