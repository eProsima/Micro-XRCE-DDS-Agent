#include "AgentSerialization.hpp"
#include <uxr/agent/types/XRCETypes.hpp>
#include <uxr/agent/message/OutputMessage.hpp>
#include "../../unittest/Common.h"

const dds::xrce::ClientKey client_key      = {{0xF1, 0xF2, 0xF3, 0xF4}};
const uint8_t session_id                   = 0x01;
const uint8_t stream_id                    = 0x04;
const uint16_t sequence_nr                 = 0x0001;

dds::xrce::MessageHeader generate_message_header()
{
    dds::xrce::MessageHeader message_header;
    message_header.client_key(client_key);
    message_header.session_id(session_id);
    message_header.stream_id(stream_id);
    message_header.sequence_nr(sequence_nr);
    return message_header;
}

std::vector<uint8_t> AgentSerialization::create_client_payload()
{
    /* Header. */
    dds::xrce::MessageHeader header = generate_message_header();

    /* Payload. */
    dds::xrce::CREATE_CLIENT_Payload payload;
    payload.client_representation().xrce_cookie() = {0x89, 0xAB, 0xCD, 0xEF};
    payload.client_representation().xrce_version() = {0x01, 0x23};
    payload.client_representation().xrce_vendor_id() = {0x45, 0x67};
    payload.client_representation().client_key() = {0x89, 0xAB, 0xCD, 0xEF};
    payload.client_representation().session_id() = 0x01;
    payload.client_representation().mtu() = 0x2345;

    /* Subheader. */
    dds::xrce::SubmessageHeader subheader;
    subheader.submessage_id(dds::xrce::CREATE_CLIENT);
    subheader.flags(0x01);
    subheader.submessage_length(uint16_t(payload.getCdrSerializedSize()));

    /* Message size. */
    size_t message_size = header.getCdrSerializedSize() +
                          subheader.getCdrSerializedSize() +
                          payload.getCdrSerializedSize();

    eprosima::uxr::OutputMessage output(header, message_size);
    output.append_submessage(dds::xrce::CREATE_CLIENT, payload, 0x0001);

    std::vector<uint8_t> buffer;
    buffer.assign(output.get_buf(), output.get_buf() + output.get_len());

    return buffer;
}

std::vector<uint8_t> AgentSerialization::create_payload()
{
    /* Header. */
    dds::xrce::MessageHeader header = generate_message_header();

    /* Payload. */
    dds::xrce::CREATE_Payload payload;
    payload.request_id() = {0x01, 0x23};
    payload.object_id() = {0x45, 0x67};
    payload.object_representation()._d() = dds::xrce::OBJK_PARTICIPANT;
    payload.object_representation().participant().representation()._d() = dds::xrce::REPRESENTATION_BY_REFERENCE;
    payload.object_representation().participant().representation().object_reference() = "ABCDE";
    payload.object_representation().participant().domain_id() = (uint16_t)0x09AB;

    /* Subheader. */
    dds::xrce::SubmessageHeader subheader;
    subheader.submessage_id(dds::xrce::CREATE);
    subheader.flags(0x01);
    subheader.submessage_length(uint16_t(payload.getCdrSerializedSize()));

    /* Message size. */
    size_t message_size = header.getCdrSerializedSize() +
                          subheader.getCdrSerializedSize() +
                          payload.getCdrSerializedSize();

    eprosima::uxr::OutputMessage output(header, message_size);
    output.append_submessage(dds::xrce::CREATE, payload, 0x0001);

    std::vector<uint8_t> buffer;
    buffer.assign(output.get_buf(), output.get_buf() + output.get_len());

    return buffer;
}

std::vector<uint8_t> AgentSerialization::get_info_payload()
{
    /* Header. */
    dds::xrce::MessageHeader header = generate_message_header();

    /* Payload. */
    dds::xrce::GET_INFO_Payload payload;
    payload.request_id() = {0x01, 0x23};
    payload.object_id() = {0x45, 0x67};
    payload.info_mask() = (dds::xrce::InfoMask)0x89ABCDEF;

    /* Subheader. */
    dds::xrce::SubmessageHeader subheader;
    subheader.submessage_id(dds::xrce::GET_INFO);
    subheader.flags(0x01);
    subheader.submessage_length(uint16_t(payload.getCdrSerializedSize()));

    /* Message size. */
    size_t message_size = header.getCdrSerializedSize() +
                          subheader.getCdrSerializedSize() +
                          payload.getCdrSerializedSize();

    eprosima::uxr::OutputMessage output(header, message_size);
    output.append_submessage(dds::xrce::GET_INFO, payload, 0x0001);

    std::vector<uint8_t> buffer;
    buffer.assign(output.get_buf(), output.get_buf() + output.get_len());

    return buffer;
}

std::vector<uint8_t> AgentSerialization::delete_payload()
{
    /* Header. */
    dds::xrce::MessageHeader header = generate_message_header();

    /* Payload. */
    dds::xrce::DELETE_Payload payload;
    payload.request_id() = {0x01, 0x23};
    payload.object_id() = {0x45, 0x67};

    /* Subheader. */
    dds::xrce::SubmessageHeader subheader;
    subheader.submessage_id(dds::xrce::DELETE_ID);
    subheader.flags(0x01);
    subheader.submessage_length(uint16_t(payload.getCdrSerializedSize()));

    /* Message size. */
    size_t message_size = header.getCdrSerializedSize() +
                          subheader.getCdrSerializedSize() +
                          payload.getCdrSerializedSize();

    eprosima::uxr::OutputMessage output(header, message_size);
    output.append_submessage(dds::xrce::DELETE_ID, payload, 0x0001);

    std::vector<uint8_t> buffer;
    buffer.assign(output.get_buf(), output.get_buf() + output.get_len());

    return buffer;
}

std::vector<uint8_t> AgentSerialization::status_agent_payload()
{
    /* Header. */
    dds::xrce::MessageHeader header = generate_message_header();

    /* Payload. */
    dds::xrce::STATUS_AGENT_Payload payload;
    payload.result().status() = dds::xrce::StatusValue(0x01);
    payload.result().implementation_status() = {0x23};
    payload.agent_info().xrce_cookie({0x89, 0xAB, 0xCD, 0xEF});
    payload.agent_info().xrce_version({0x01, 0x23});
    payload.agent_info().xrce_vendor_id({0x45, 0x67});

    /* Subheader. */
    dds::xrce::SubmessageHeader subheader;
    subheader.submessage_id(dds::xrce::STATUS_AGENT);
    subheader.flags(0x01);
    subheader.submessage_length(uint16_t(payload.getCdrSerializedSize()));

    /* Message size. */
    size_t message_size = header.getCdrSerializedSize() +
                          subheader.getCdrSerializedSize() +
                          payload.getCdrSerializedSize();

    eprosima::uxr::OutputMessage output(header, message_size);
    output.append_submessage(dds::xrce::STATUS_AGENT, payload, 0x0001);

    std::vector<uint8_t> buffer;
    buffer.assign(output.get_buf(), output.get_buf() + output.get_len());

    return buffer;
}

std::vector<uint8_t> AgentSerialization::status_payload()
{
    /* Header. */
    dds::xrce::MessageHeader header = generate_message_header();

    /* Payload. */
    dds::xrce::STATUS_Payload payload;
    payload.related_request().request_id() = {0x01, 0x23};
    payload.related_request().object_id() = {0x45, 0x67};
    payload.result().implementation_status() = 0x89;
    payload.result().status() = dds::xrce::StatusValue(0xAB);

    /* Subheader. */
    dds::xrce::SubmessageHeader subheader;
    subheader.submessage_id(dds::xrce::STATUS);
    subheader.flags(0x01);
    subheader.submessage_length(uint16_t(payload.getCdrSerializedSize()));

    /* Message size. */
    size_t message_size = header.getCdrSerializedSize() +
                          subheader.getCdrSerializedSize() +
                          payload.getCdrSerializedSize();

    eprosima::uxr::OutputMessage output(header, message_size);
    output.append_submessage(dds::xrce::STATUS, payload, 0x0001);

    std::vector<uint8_t> buffer;
    buffer.assign(output.get_buf(), output.get_buf() + output.get_len());

    return buffer;
}

std::vector<uint8_t> AgentSerialization::info_payload()
{
    /* Header. */
    dds::xrce::MessageHeader header = generate_message_header();

    /* Payload. */
    dds::xrce::TransportAddressMedium medium;
    medium.address() = {0x01, 0x23, 0x45, 0x67};
    medium.port() = 0x0123;

    dds::xrce::TransportAddress address;
    address.medium_locator(medium);

    dds::xrce::AGENT_ActivityInfo agent_activity;
    agent_activity.address_seq().push_back(address);
    agent_activity.availability() = 1;

    dds::xrce::ActivityInfoVariant activity;
    activity.agent(agent_activity);

    dds::xrce::AGENT_Representation agent_config;
    agent_config.xrce_cookie({0x89, 0xAB, 0xCD, 0xEF});
    agent_config.xrce_version({0x01, 0x23});
    agent_config.xrce_vendor_id({0x45, 0x67});

    dds::xrce::ObjectVariant config;
    config.agent(agent_config);

    dds::xrce::INFO_Payload payload;
    payload.related_request().request_id() = {0x01, 0x23};
    payload.related_request().object_id() = {0x45, 0x67};
    payload.result().implementation_status() = 0x89;
    payload.result().status() = dds::xrce::StatusValue(0xAB);
    payload.object_info().activity(activity);
    payload.object_info().config(config);

    /* Subheader. */
    dds::xrce::SubmessageHeader subheader;
    subheader.submessage_id(dds::xrce::INFO);
    subheader.flags(0x01);
    subheader.submessage_length(uint16_t(payload.getCdrSerializedSize()));

    /* Message size. */
    size_t message_size = header.getCdrSerializedSize() +
                          subheader.getCdrSerializedSize() +
                          payload.getCdrSerializedSize();

    eprosima::uxr::OutputMessage output(header, message_size);
    output.append_submessage(dds::xrce::INFO, payload, 0x0001);

    std::vector<uint8_t> buffer;
    buffer.assign(output.get_buf(), output.get_buf() + output.get_len());

    return buffer;
}

std::vector<uint8_t> AgentSerialization::read_data_payload()
{
    /* Header. */
    dds::xrce::MessageHeader header = generate_message_header();

    /* Payload. */
    dds::xrce::READ_DATA_Payload payload;
    payload.request_id() = {0x01, 0x23};
    payload.object_id() = {0x45, 0x67};
    payload.read_specification().preferred_stream_id() = 0x80;
    payload.read_specification().data_format() = 0x89;

    dds::xrce::DataDeliveryControl delivery_control;
    delivery_control.max_bytes_per_second() = 0xABCD;
    delivery_control.max_elapsed_time() = 0x2345;
    delivery_control.max_samples() = 0xABCD;
    delivery_control.min_pace_period() = 0xEF01;
    payload.read_specification().delivery_control(delivery_control);
    payload.read_specification().content_filter_expression("ABCDE");

    /* Subheader. */
    dds::xrce::SubmessageHeader subheader;
    subheader.submessage_id(dds::xrce::READ_DATA);
    subheader.flags(0x01);
    subheader.submessage_length(uint16_t(payload.getCdrSerializedSize()));

    /* Message size. */
    size_t message_size = header.getCdrSerializedSize() +
                          subheader.getCdrSerializedSize() +
                          payload.getCdrSerializedSize();

    eprosima::uxr::OutputMessage output(header, message_size);
    output.append_submessage(dds::xrce::READ_DATA, payload, 0x0001);

    std::vector<uint8_t> buffer;
    buffer.assign(output.get_buf(), output.get_buf() + output.get_len());

    return buffer;
}

std::vector<uint8_t> AgentSerialization::write_data_payload_data()
{
    /* Header. */
    dds::xrce::MessageHeader header = generate_message_header();

    /* Payload. */
    dds::xrce::WRITE_DATA_Payload_Data payload;
    payload.request_id() = {0x01, 0x23};
    payload.object_id() = {0x45, 0x67};
    payload.data().serialized_data() = {'B', 'Y', 'T', 'E', 'S'};

    /* Subheader. */
    dds::xrce::SubmessageHeader subheader;
    subheader.submessage_id(dds::xrce::WRITE_DATA);
    subheader.flags(0x01);
    subheader.submessage_length(uint16_t(payload.getCdrSerializedSize()));

    /* Message size. */
    size_t message_size = header.getCdrSerializedSize() +
                          subheader.getCdrSerializedSize() +
                          payload.getCdrSerializedSize();

    eprosima::uxr::OutputMessage output(header, message_size);
    output.append_submessage(dds::xrce::WRITE_DATA, payload, 0x0001);

    std::vector<uint8_t> buffer;
    buffer.assign(output.get_buf(), output.get_buf() + output.get_len());

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
    /* Header. */
    dds::xrce::MessageHeader header = generate_message_header();

    /* Payload. */
    dds::xrce::DATA_Payload_Data payload;
    payload.request_id() = {0x01, 0x23};
    payload.object_id() = {0x45, 0x67};
    payload.data().serialized_data() = {'B', 'Y', 'T', 'E', 'S'};

    /* Subheader. */
    dds::xrce::SubmessageHeader subheader;
    subheader.submessage_id(dds::xrce::DATA);
    subheader.flags(0x01);
    subheader.submessage_length(uint16_t(payload.getCdrSerializedSize()));

    /* Message size. */
    size_t message_size = header.getCdrSerializedSize() +
                          subheader.getCdrSerializedSize() +
                          payload.getCdrSerializedSize();

    eprosima::uxr::OutputMessage output(header, message_size);
    output.append_submessage(dds::xrce::DATA, payload, 0x0001);

    std::vector<uint8_t> buffer;
    buffer.assign(output.get_buf(), output.get_buf() + output.get_len());

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
    /* Header. */
    dds::xrce::MessageHeader header = generate_message_header();

    /* Payload. */
    dds::xrce::ACKNACK_Payload payload;
    payload.first_unacked_seq_num() = (uint16_t)0x0123;
    payload.nack_bitmap() = {0x45, 0x67};
    payload.stream_id() = (uint8_t)0x89;

    /* Subheader. */
    dds::xrce::SubmessageHeader subheader;
    subheader.submessage_id(dds::xrce::ACKNACK);
    subheader.flags(0x01);
    subheader.submessage_length(uint16_t(payload.getCdrSerializedSize()));

    /* Message size. */
    size_t message_size = header.getCdrSerializedSize() +
                          subheader.getCdrSerializedSize() +
                          payload.getCdrSerializedSize();

    eprosima::uxr::OutputMessage output(header, message_size);
    output.append_submessage(dds::xrce::ACKNACK, payload, 0x0001);

    std::vector<uint8_t> buffer;
    buffer.assign(output.get_buf(), output.get_buf() + output.get_len());

    return buffer;
}

std::vector<uint8_t> AgentSerialization::heartbeat_payload()
{
    /* Header. */
    dds::xrce::MessageHeader header = generate_message_header();

    /* Payload. */
    dds::xrce::HEARTBEAT_Payload payload;
    payload.first_unacked_seq_nr() = (uint16_t)0x0123;
    payload.last_unacked_seq_nr() = (uint16_t)0x4567;
    payload.stream_id() = (uint8_t)0x89;

    /* Subheader. */
    dds::xrce::SubmessageHeader subheader;
    subheader.submessage_id(dds::xrce::HEARTBEAT);
    subheader.flags(0x01);
    subheader.submessage_length(uint16_t(payload.getCdrSerializedSize()));

    /* Message size. */
    size_t message_size = header.getCdrSerializedSize() +
                          subheader.getCdrSerializedSize() +
                          payload.getCdrSerializedSize();

    eprosima::uxr::OutputMessage output(header, message_size);
    output.append_submessage(dds::xrce::HEARTBEAT, payload, 0x0001);

    std::vector<uint8_t> buffer;
    buffer.assign(output.get_buf(), output.get_buf() + output.get_len());

    return buffer;
}
