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
#include <uxr/agent/Root.hpp>

namespace eprosima {
namespace uxr {

//static uint32_t clientkey_to_raw(const dds::xrce::ClientKey& client_key)
//{
//    return uint32_t(client_key[0] << 24)
//         + uint32_t(client_key[1] << 16)
//         + uint32_t(client_key[2] << 8)
//         + uint32_t(client_key[3]);
//}

static dds::xrce::ClientKey raw_to_clientkey(uint32_t key)
{
    dds::xrce::ClientKey client_key{
        uint8_t(key >> 24),
        uint8_t(key >> 16),
        uint8_t(key >> 8),
        uint8_t(key)};
    return client_key;
}

/**********************************************************************************************************************
 * Client.
 **********************************************************************************************************************/
bool Agent::create_client(
        uint32_t key,
        uint8_t session,
        uint16_t mtu,
        ResultStatus& errcode)
{
    Root& root = Root::instance();
    dds::xrce::CLIENT_Representation client_representation;
    dds::xrce::AGENT_Representation agent_representation;
    dds::xrce::ResultStatus result;

    client_representation.client_key(raw_to_clientkey(key));
    client_representation.xrce_cookie(dds::xrce::XRCE_COOKIE);
    client_representation.xrce_version(dds::xrce::XRCE_VERSION);
    client_representation.session_id(session);
    client_representation.mtu(mtu);
    result = root.create_client(client_representation, agent_representation);
    if (dds::xrce::STATUS_OK != result.status())
    {
        errcode = ResultStatus(result.status());
    }

    return (dds::xrce::STATUS_OK == result.status());
}

bool Agent::delete_client(
        uint32_t key,
        ResultStatus& errcode)
{
    Root& root = Root::instance();
    dds::xrce::ResultStatus result = root.delete_client(raw_to_clientkey(key));

    if (dds::xrce::STATUS_OK != result.status())
    {
        errcode = ResultStatus(result.status());
    }

    return (dds::xrce::STATUS_OK == result.status());
}

/**********************************************************************************************************************
 * Participant.
 **********************************************************************************************************************/
bool Agent::create_participant_by_ref(
        uint32_t client_key,
        uint16_t participant_id,
        int16_t domain_id,
        const char* ref,
        uint8_t flag,
        ResultStatus& errcode)
{
    bool rv = false;
    Root& root = Root::instance();

    if (std::shared_ptr<ProxyClient> client = root.get_client(raw_to_clientkey(client_key)))
    {
        if (dds::xrce::OBJK_PARTICIPANT == (participant_id & 0x000F))
        {
            dds::xrce::CreationMode creation_mode{}; // TODO (julian): provide flag argument.
            dds::xrce::ObjectId object_id = XRCEObject::raw_to_objectid(participant_id);
            dds::xrce::ObjectVariant object_variant;
            dds::xrce::OBJK_PARTICIPANT_Representation participant;

            creation_mode.reuse(0 != (flag & REUSE_MODE));
            creation_mode.replace(0 != (flag & REPLACE_MODE));
            participant.domain_id(domain_id);
            participant.representation().object_reference(ref);
            object_variant.participant(participant);

            dds::xrce::ResultStatus result = client->create(creation_mode, object_id, object_variant);
            errcode = ResultStatus(result.status());
            rv = (dds::xrce::STATUS_OK == result.status() || dds::xrce::STATUS_OK_MATCHED == result.status());
        }
        else
        {
            errcode = ResultStatus(dds::xrce::STATUS_ERR_INVALID_DATA);
        }
    }
    else
    {
        errcode = ResultStatus(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
    }

    return rv;
}

bool Agent::create_participant_by_xml(
        uint32_t client_key,
        uint16_t participant_id,
        int16_t domain_id,
        const char* xml,
        uint8_t flag,
        ResultStatus& errcode)
{
    bool rv = false;
    Root& root = Root::instance();

    if (std::shared_ptr<ProxyClient> client = root.get_client(raw_to_clientkey(client_key)))
    {
        if (dds::xrce::OBJK_PARTICIPANT == (participant_id & 0x000F))
        {
            dds::xrce::CreationMode creation_mode{}; // TODO (julian): provide flag argument.
            dds::xrce::ObjectId object_id = XRCEObject::raw_to_objectid(participant_id);
            dds::xrce::ObjectVariant object_variant;
            dds::xrce::OBJK_PARTICIPANT_Representation participant;

            creation_mode.reuse(0 != (flag & REUSE_MODE));
            creation_mode.replace(0 != (flag & REPLACE_MODE));
            participant.domain_id(domain_id);
            participant.representation().xml_string_representation(xml);
            object_variant.participant(participant);

            dds::xrce::ResultStatus result = client->create(creation_mode, object_id, object_variant);
            errcode = ResultStatus(result.status());
            rv = (dds::xrce::STATUS_OK == result.status() || dds::xrce::STATUS_OK_MATCHED == result.status());
        }
        else
        {
            errcode = ResultStatus(dds::xrce::STATUS_ERR_INVALID_DATA);
        }
    }
    else
    {
        errcode = ResultStatus(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
    }

    return rv;
}

/**********************************************************************************************************************
 * Topic.
 **********************************************************************************************************************/
bool Agent::create_topic_by_ref(
        uint32_t client_key,
        uint16_t topic_id,
        uint16_t participant_id,
        const char* ref,
        uint8_t flag,
        ResultStatus& errcode)
{
    bool rv = false;
    Root& root = Root::instance();

    if (std::shared_ptr<ProxyClient> client = root.get_client(raw_to_clientkey(client_key)))
    {
        if (dds::xrce::OBJK_TOPIC == (topic_id & 0x000F))
        {
            dds::xrce::CreationMode creation_mode{}; // TODO (julian): provide flag argument.
            dds::xrce::ObjectId object_id = XRCEObject::raw_to_objectid(topic_id);
            dds::xrce::ObjectVariant object_variant;
            dds::xrce::OBJK_TOPIC_Representation topic;

            creation_mode.reuse(0 != (flag & REUSE_MODE));
            creation_mode.replace(0 != (flag & REPLACE_MODE));
            topic.participant_id(XRCEObject::raw_to_objectid(participant_id));
            topic.representation().object_reference(ref);
            object_variant.topic(topic);

            dds::xrce::ResultStatus result = client->create(creation_mode, object_id, object_variant);
            errcode = ResultStatus(result.status());
            rv = (dds::xrce::STATUS_OK == result.status() || dds::xrce::STATUS_OK_MATCHED == result.status());
        }
        else
        {
            errcode = ResultStatus(dds::xrce::STATUS_ERR_INVALID_DATA);
        }
    }
    else
    {
        errcode = ResultStatus(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
    }

    return rv;
}

bool Agent::create_topic_by_xml(
        uint32_t client_key,
        uint16_t topic_id,
        uint16_t participant_id,
        const char* xml,
        uint8_t flag,
        ResultStatus& errcode)
{
    bool rv = false;
    Root& root = Root::instance();

    if (std::shared_ptr<ProxyClient> client = root.get_client(raw_to_clientkey(client_key)))
    {
        if (dds::xrce::OBJK_TOPIC == (topic_id & 0x000F))
        {
            dds::xrce::CreationMode creation_mode{}; // TODO (julian): provide flag argument.
            dds::xrce::ObjectId object_id = XRCEObject::raw_to_objectid(topic_id);
            dds::xrce::ObjectVariant object_variant;
            dds::xrce::OBJK_TOPIC_Representation topic;

            creation_mode.reuse(0 != (flag & REUSE_MODE));
            creation_mode.replace(0 != (flag & REPLACE_MODE));
            topic.participant_id(XRCEObject::raw_to_objectid(participant_id));
            topic.representation().xml_string_representation(xml);
            object_variant.topic(topic);

            dds::xrce::ResultStatus result = client->create(creation_mode, object_id, object_variant);
            errcode = ResultStatus(result.status());
            rv = (dds::xrce::STATUS_OK == result.status() || dds::xrce::STATUS_OK_MATCHED == result.status());
        }
        else
        {
            errcode = ResultStatus(dds::xrce::STATUS_ERR_INVALID_DATA);
        }
    }
    else
    {
        errcode = ResultStatus(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
    }

    return rv;
}

/**********************************************************************************************************************
 * Publisher.
 **********************************************************************************************************************/
bool Agent::create_publisher_by_xml(
        uint32_t client_key,
        uint16_t publisher_id,
        uint16_t participant_id,
        const char* xml,
        uint8_t flag,
        ResultStatus& errcode)
{
    bool rv = false;
    Root& root = Root::instance();

    if (std::shared_ptr<ProxyClient> client = root.get_client(raw_to_clientkey(client_key)))
    {
        if (dds::xrce::OBJK_PUBLISHER == (publisher_id & 0x000F))
        {
            dds::xrce::CreationMode creation_mode{}; // TODO (julian): provide flag argument.
            dds::xrce::ObjectId object_id = XRCEObject::raw_to_objectid(publisher_id);
            dds::xrce::ObjectVariant object_variant;
            dds::xrce::OBJK_PUBLISHER_Representation publisher;

            creation_mode.reuse(0 != (flag & REUSE_MODE));
            creation_mode.replace(0 != (flag & REPLACE_MODE));
            publisher.participant_id(XRCEObject::raw_to_objectid(participant_id));
            publisher.representation().string_representation(xml);
            object_variant.publisher(publisher);

            dds::xrce::ResultStatus result = client->create(creation_mode, object_id, object_variant);
            errcode = ResultStatus(result.status());
            rv = (dds::xrce::STATUS_OK == result.status() || dds::xrce::STATUS_OK_MATCHED == result.status());
        }
        else
        {
            errcode = ResultStatus(dds::xrce::STATUS_ERR_INVALID_DATA);
        }
    }
    else
    {
        errcode = ResultStatus(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
    }

    return rv;
}

/**********************************************************************************************************************
 * Subscriber.
 **********************************************************************************************************************/
bool Agent::create_subscriber_by_xml(
        uint32_t client_key,
        uint16_t subscriber_id,
        uint16_t participant_id,
        const char* xml,
        uint8_t flag,
        ResultStatus& errcode)
{
    bool rv = false;
    Root& root = Root::instance();

    if (std::shared_ptr<ProxyClient> client = root.get_client(raw_to_clientkey(client_key)))
    {
        if (dds::xrce::OBJK_SUBSCRIBER == (subscriber_id & 0x000F))
        {
            dds::xrce::CreationMode creation_mode{}; // TODO (julian): provide flag argument.
            dds::xrce::ObjectId object_id = XRCEObject::raw_to_objectid(subscriber_id);
            dds::xrce::ObjectVariant object_variant;
            dds::xrce::OBJK_SUBSCRIBER_Representation subscriber;

            creation_mode.reuse(0 != (flag & REUSE_MODE));
            creation_mode.replace(0 != (flag & REPLACE_MODE));
            subscriber.participant_id(XRCEObject::raw_to_objectid(participant_id));
            subscriber.representation().string_representation(xml);
            object_variant.subscriber(subscriber);

            dds::xrce::ResultStatus result = client->create(creation_mode, object_id, object_variant);
            errcode = ResultStatus(result.status());
            rv = (dds::xrce::STATUS_OK == result.status() || dds::xrce::STATUS_OK_MATCHED == result.status());
        }
        else
        {
            errcode = ResultStatus(dds::xrce::STATUS_ERR_INVALID_DATA);
        }
    }
    else
    {
        errcode = ResultStatus(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
    }

    return rv;
}

/**********************************************************************************************************************
 * DataWriter.
 **********************************************************************************************************************/
bool Agent::create_datawriter_by_ref(
        uint32_t client_key,
        uint16_t datawriter_id,
        uint16_t publisher_id,
        const char* ref,
        uint8_t flag,
        ResultStatus& errcode)
{
    bool rv = false;
    Root& root = Root::instance();

    if (std::shared_ptr<ProxyClient> client = root.get_client(raw_to_clientkey(client_key)))
    {
        if (dds::xrce::OBJK_DATAWRITER == (datawriter_id & 0x000F))
        {
            dds::xrce::CreationMode creation_mode{}; // TODO (julian): provide flag argument.
            dds::xrce::ObjectId object_id = XRCEObject::raw_to_objectid(datawriter_id);
            dds::xrce::ObjectVariant object_variant;
            dds::xrce::DATAWRITER_Representation datawriter;

            creation_mode.reuse(0 != (flag & REUSE_MODE));
            creation_mode.replace(0 != (flag & REPLACE_MODE));
            datawriter.publisher_id(XRCEObject::raw_to_objectid(publisher_id));
            datawriter.representation().object_reference(ref);
            object_variant.data_writer(datawriter);

            dds::xrce::ResultStatus result = client->create(creation_mode, object_id, object_variant);
            errcode = ResultStatus(result.status());
            rv = (dds::xrce::STATUS_OK == result.status() || dds::xrce::STATUS_OK_MATCHED == result.status());
        }
        else
        {
            errcode = ResultStatus(dds::xrce::STATUS_ERR_INVALID_DATA);
        }
    }
    else
    {
        errcode = ResultStatus(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
    }

    return rv;
}

bool Agent::create_datawriter_by_xml(
        uint32_t client_key,
        uint16_t datawriter_id,
        uint16_t publisher_id,
        const char* xml,
        uint8_t flag,
        ResultStatus& errcode)
{
    bool rv = false;
    Root& root = Root::instance();

    if (std::shared_ptr<ProxyClient> client = root.get_client(raw_to_clientkey(client_key)))
    {
        if (dds::xrce::OBJK_DATAWRITER == (datawriter_id & 0x000F))
        {
            dds::xrce::CreationMode creation_mode{}; // TODO (julian): provide flag argument.
            dds::xrce::ObjectId object_id = XRCEObject::raw_to_objectid(datawriter_id);
            dds::xrce::ObjectVariant object_variant;
            dds::xrce::DATAWRITER_Representation datawriter;

            creation_mode.reuse(0 != (flag & REUSE_MODE));
            creation_mode.replace(0 != (flag & REPLACE_MODE));
            datawriter.publisher_id(XRCEObject::raw_to_objectid(publisher_id));
            datawriter.representation().xml_string_representation(xml);
            object_variant.data_writer(datawriter);

            dds::xrce::ResultStatus result = client->create(creation_mode, object_id, object_variant);
            errcode = ResultStatus(result.status());
            rv = (dds::xrce::STATUS_OK == result.status() || dds::xrce::STATUS_OK_MATCHED == result.status());
        }
        else
        {
            errcode = ResultStatus(dds::xrce::STATUS_ERR_INVALID_DATA);
        }
    }
    else
    {
        errcode = ResultStatus(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
    }

    return rv;
}

/**********************************************************************************************************************
 * DataReader.
 **********************************************************************************************************************/
bool Agent::create_datareader_by_ref(
        uint32_t client_key,
        uint16_t datareader_id,
        uint16_t subscriber_id,
        const char* ref,
        uint8_t flag,
        ResultStatus& errcode)
{
    bool rv = false;
    Root& root = Root::instance();

    if (std::shared_ptr<ProxyClient> client = root.get_client(raw_to_clientkey(client_key)))
    {
        if (dds::xrce::OBJK_DATAREADER == (datareader_id & 0x000F))
        {
            dds::xrce::CreationMode creation_mode{}; // TODO (julian): provide flag argument.
            dds::xrce::ObjectId object_id = XRCEObject::raw_to_objectid(datareader_id);
            dds::xrce::ObjectVariant object_variant;
            dds::xrce::DATAREADER_Representation datareader;

            creation_mode.reuse(0 != (flag & REUSE_MODE));
            creation_mode.replace(0 != (flag & REPLACE_MODE));
            datareader.subscriber_id(XRCEObject::raw_to_objectid(subscriber_id));
            datareader.representation().object_reference(ref);
            object_variant.data_reader(datareader);

            dds::xrce::ResultStatus result = client->create(creation_mode, object_id, object_variant);
            errcode = ResultStatus(result.status());
            rv = (dds::xrce::STATUS_OK == result.status() || dds::xrce::STATUS_OK_MATCHED == result.status());
        }
        else
        {
            errcode = ResultStatus(dds::xrce::STATUS_ERR_INVALID_DATA);
        }
    }
    else
    {
        errcode = ResultStatus(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
    }

    return rv;
}

bool Agent::create_datareader_by_xml(
        uint32_t client_key,
        uint16_t datareader_id,
        uint16_t subscriber_id,
        const char* xml,
        uint8_t flag,
        ResultStatus& errcode)
{
    bool rv = false;
    Root& root = Root::instance();

    if (std::shared_ptr<ProxyClient> client = root.get_client(raw_to_clientkey(client_key)))
    {
        if (dds::xrce::OBJK_DATAREADER == (datareader_id & 0x000F))
        {
            dds::xrce::CreationMode creation_mode{}; // TODO (julian): provide flag argument.
            dds::xrce::ObjectId object_id = XRCEObject::raw_to_objectid(datareader_id);
            dds::xrce::ObjectVariant object_variant;
            dds::xrce::DATAREADER_Representation datareader;

            creation_mode.reuse(0 != (flag & REUSE_MODE));
            creation_mode.replace(0 != (flag & REPLACE_MODE));
            datareader.subscriber_id(XRCEObject::raw_to_objectid(subscriber_id));
            datareader.representation().xml_string_representation(xml);
            object_variant.data_reader(datareader);

            dds::xrce::ResultStatus result = client->create(creation_mode, object_id, object_variant);
            errcode = ResultStatus(result.status());
            rv = (dds::xrce::STATUS_OK == result.status() || dds::xrce::STATUS_OK_MATCHED == result.status());
        }
        else
        {
            errcode = ResultStatus(dds::xrce::STATUS_ERR_INVALID_DATA);
        }
    }
    else
    {
        errcode = ResultStatus(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
    }

    return rv;
}

/**********************************************************************************************************************
 * Delete Object.
 **********************************************************************************************************************/
bool Agent::delete_object(
        uint32_t client_key,
        uint16_t object_id,
        ResultStatus& errcode)
{
    bool rv = false;
    Root& root = Root::instance();

    if (std::shared_ptr<ProxyClient> client = root.get_client(raw_to_clientkey(client_key)))
    {
        dds::xrce::ResultStatus result = client->delete_object(XRCEObject::raw_to_objectid(object_id));
        errcode = ResultStatus(result.status());
        rv = (dds::xrce::STATUS_OK == result.status() || dds::xrce::STATUS_OK_MATCHED == result.status());
    }
    else
    {
        errcode = ResultStatus(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
    }

    return rv;
}

/**********************************************************************************************************************
 * Reset.
 **********************************************************************************************************************/
void Agent::reset()
{
    Root::instance().reset();
}

} // namespace uxr
} // namespace eprosima
