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

#include <uxr/agent/Agent.hpp>

namespace eprosima {
namespace uxr {

bool Agent::create_client(
        uint32_t key,
        uint8_t session,
        uint8_t& errcode)
{
    return false;
}

bool Agent::create_participant_by_ref(
        uint32_t client_key,
        uint16_t participant_id,
        int16_t domain_id,
        const char* ref,
        uint8_t& errcode)
{
    return false;
}

bool Agent::create_participant_by_xml(
        uint32_t client_key,
        uint16_t participant_id,
        int16_t domain_id,
        const char* xml,
        uint8_t& errcode)
{
    return false;
}

bool Agent::create_topic_by_ref(
        uint32_t client_key,
        uint16_t topic_id,
        uint16_t participant_id,
        const char* ref,
        uint8_t& errcode)
{
    return false;
}

bool Agent::create_topic_by_xml(
        uint32_t client_key,
        uint16_t topic_id,
        uint16_t participant_id,
        const char* xml,
        uint8_t& errcode)
{
    return false;
}

bool Agent::create_publisher_by_ref(
        uint32_t client_key,
        uint16_t publisher_id,
        uint16_t participant_id,
        const char* ref,
        uint8_t& errcode)
{
    return false;
}

bool Agent::create_publisher_by_xml(
        uint32_t client_key,
        uint16_t publisher_id,
        uint16_t participant_id,
        const char* xml,
        uint8_t& errcode)
{
    return false;
}

bool Agent::create_subscriber_by_ref(
        uint32_t client_key,
        uint16_t subscriber_id,
        uint16_t participant_id,
        const char* ref,
        uint8_t& errcode)
{
    return false;
}

bool Agent::create_subscriber_by_xml(
        uint32_t client_key,
        uint16_t subscriber_id,
        uint16_t participant_id,
        const char* xml,
        uint8_t& errcode)
{
    return false;
}

bool Agent::create_datawriter_by_ref(
        uint32_t client_key,
        uint16_t datawriter_id,
        uint16_t publisher_id,
        const char* ref,
        uint8_t& errcode)
{
    return false;
}

bool Agent::create_datawriter_by_xml(
        uint32_t client_key,
        uint16_t datawriter_id,
        uint16_t publisher_id,
        const char* xml,
        uint8_t& errcode)
{
    return false;
}

bool Agent::create_datareader_by_ref(
        uint32_t client_key,
        uint16_t datareader_id,
        uint16_t subscriber_id,
        const char* ref,
        uint8_t& errcode)
{
    return false;
}

bool Agent::create_datareader_by_xml(
        uint32_t client_key,
        uint16_t datareader_id,
        uint16_t subscriber_id,
        const char* xml,
        uint8_t& errcode)
{
    return false;
}

} // namespace uxr
} // namespace eprosima
