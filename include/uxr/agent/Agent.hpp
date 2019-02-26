// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef UXR_AGENT_AGENT_HPP_
#define UXR_AGENT_AGENT_HPP_

#include <uxr/agent/agent_dll.hpp>

#include <cstdint>

namespace eprosima{
namespace uxr{


class Agent
{
public:
    enum ErrorCode : uint8_t
    {
        OK_ERRCODE                  = 0x00,
        OK_MATCHED_ERRCODE          = 0x01,
        DDS_ERROR_ERRCODE           = 0x80,
        MISMATCH_ERRCODE            = 0x81,
        ALREADY_EXISTS_ERRCODE      = 0x82,
        DENIED_ERRCODE              = 0x83,
        UNKNOWN_REFERENCE_ERRCODE   = 0x84,
        INVALID_DATA_ERRCODE        = 0x85,
        INCOMPATIBLE_ERRCODE        = 0x86,
        RESOURCES_ERRCODE           = 0x87
    };

    enum CreationFlag : uint8_t
    {
        REUSE_MODE      = 0x01 << 1,
        REPLACE_MODE    = 0x01 << 2
    };

    enum ObjectKind : uint8_t
    {
        PARTICIPANT_OBJK    = 0x01,
        TOPIC_OBJK          = 0x02,
        PUBLISHER_OBJK      = 0x03,
        SUBSCRIBER_OBJK     = 0x04,
        DATAWRITER_OBJK     = 0x05,
        DATAREADER_OBJK     = 0x06
    };

    static uint16_t get_object_id(uint16_t prefix, ObjectKind object_kind)
    {
        return uint16_t((uint16_t(prefix) << 4) + object_kind);
    }

    Agent() = delete;
    ~Agent() = delete;

    /**********************************************************************************************
     * Client.
     **********************************************************************************************/
    microxrcedds_agent_DllAPI static bool create_client(
            uint32_t key,
            uint8_t session,
            uint16_t mtu,
            ErrorCode& errcode);

    microxrcedds_agent_DllAPI static bool delete_client(
            uint32_t key,
            ErrorCode& errcode);

    /**********************************************************************************************
     * Participant.
     **********************************************************************************************/
    microxrcedds_agent_DllAPI static bool create_participant_by_ref(
            uint32_t client_key,
            uint16_t participant_id,
            int16_t domain_id,
            const char* ref,
            uint8_t flag,
            ErrorCode& errcode);

    microxrcedds_agent_DllAPI static bool create_participant_by_xml(
            uint32_t client_key,
            uint16_t participant_id,
            int16_t domain_id,
            const char* xml,
            uint8_t flag,
            ErrorCode& errcode);

    /**********************************************************************************************
     * Topic.
     **********************************************************************************************/
    microxrcedds_agent_DllAPI static bool create_topic_by_ref(
            uint32_t client_key,
            uint16_t topic_id,
            uint16_t participant_id,
            const char* ref,
            uint8_t flag,
            ErrorCode& errcode);

    microxrcedds_agent_DllAPI static bool create_topic_by_xml(
            uint32_t client_key,
            uint16_t topic_id,
            uint16_t participant_id,
            const char* xml,
            uint8_t flag,
            ErrorCode& errcode);

    /**********************************************************************************************
     * Publisher.
     **********************************************************************************************/
    microxrcedds_agent_DllAPI static bool create_publisher_by_xml(
            uint32_t client_key,
            uint16_t publisher_id,
            uint16_t participant_id,
            const char* xml,
            uint8_t flag,
            ErrorCode& errcode);

    /**********************************************************************************************
     * Subscriber.
     **********************************************************************************************/
    microxrcedds_agent_DllAPI static bool create_subscriber_by_xml(
            uint32_t client_key,
            uint16_t subscriber_id,
            uint16_t participant_id,
            const char* xml,
            uint8_t flag,
            ErrorCode& errcode);

    /**********************************************************************************************
     * DataWriter.
     **********************************************************************************************/
    microxrcedds_agent_DllAPI static bool create_datawriter_by_ref(
            uint32_t client_key,
            uint16_t datawriter_id,
            uint16_t publisher_id,
            const char* ref,
            uint8_t flag,
            ErrorCode& errcode);

    microxrcedds_agent_DllAPI static bool create_datawriter_by_xml(
            uint32_t client_key,
            uint16_t datawriter_id,
            uint16_t publisher_id,
            const char* xml,
            uint8_t flag,
            ErrorCode& errcode);

    /**********************************************************************************************
     * DataReader.
     **********************************************************************************************/
    microxrcedds_agent_DllAPI static bool create_datareader_by_ref(
            uint32_t client_key,
            uint16_t datareader_id,
            uint16_t subscriber_id,
            const char* ref,
            uint8_t flag,
            ErrorCode& errcode);

    microxrcedds_agent_DllAPI static bool create_datareader_by_xml(
            uint32_t client_key,
            uint16_t datareader_id,
            uint16_t subscriber_id,
            const char* xml,
            uint8_t flag,
            ErrorCode& errcode);

    /**********************************************************************************************
     * Delete Object.
     **********************************************************************************************/
    microxrcedds_agent_DllAPI static bool delete_object(
            uint32_t client_key,
            uint16_t object_id,
            ErrorCode& errcode);

    /**********************************************************************************************
     * Reset.
     **********************************************************************************************/
    microxrcedds_agent_DllAPI static void reset();
};

} // uxr
} // eprosima

#endif // UXR_AGENT_AGENT_HPP_
