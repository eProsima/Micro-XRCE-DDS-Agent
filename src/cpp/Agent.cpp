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
#include <uxr/agent/utils/Convertion.hpp>
#include <uxr/agent/datawriter/DataWriter.hpp>


namespace eprosima {
namespace uxr {

namespace {

inline dds::xrce::ClientKey raw_to_clientkey(uint32_t key)
{
    dds::xrce::ClientKey client_key{
        uint8_t(key >> 24),
        uint8_t(key >> 16),
        uint8_t(key >> 8),
        uint8_t(key)};
    return client_key;
}

struct RefRep
{
    const char* ref;
};

struct XmlRep
{
    const char* xml;
};

template<Agent::ObjectKind object_kind, typename U, typename T>
void fill_object_variant(
        T parent_id,
        U rep,
        dds::xrce::ObjectVariant& object_variant);

template<>
void fill_object_variant<Agent::PARTICIPANT_OBJK>(
        int16_t domain_id,
        XmlRep rep,
        dds::xrce::ObjectVariant& object_variant)
{
    dds::xrce::OBJK_PARTICIPANT_Representation participant;
    participant.domain_id(domain_id);
    participant.representation().xml_string_representation(rep.xml);
    object_variant.participant(participant);
}

template<>
void fill_object_variant<Agent::PARTICIPANT_OBJK>(
        int16_t domain_id,
        RefRep rep,
        dds::xrce::ObjectVariant& object_variant)
{
    dds::xrce::OBJK_PARTICIPANT_Representation participant;
    participant.domain_id(domain_id);
    participant.representation().object_reference(rep.ref);
    object_variant.participant(participant);
}

template<>
void fill_object_variant<Agent::TOPIC_OBJK>(
        uint16_t participant_id,
        XmlRep rep,
        dds::xrce::ObjectVariant& object_variant)
{
    dds::xrce::OBJK_TOPIC_Representation topic;
    topic.participant_id(convertion::raw_to_objectid(participant_id));
    topic.representation().xml_string_representation(rep.xml);
    object_variant.topic(topic);
}

template<>
void fill_object_variant<Agent::TOPIC_OBJK>(
        uint16_t participant_id,
        RefRep rep,
        dds::xrce::ObjectVariant& object_variant)
{
    dds::xrce::OBJK_TOPIC_Representation topic;
    topic.participant_id(convertion::raw_to_objectid(participant_id));
    topic.representation().object_reference(rep.ref);
    object_variant.topic(topic);
}

template<>
void fill_object_variant<Agent::PUBLISHER_OBJK>(
        uint16_t participant_id,
        XmlRep rep,
        dds::xrce::ObjectVariant& object_variant)
{
    dds::xrce::OBJK_PUBLISHER_Representation publisher;
    publisher.participant_id(convertion::raw_to_objectid(participant_id));
    publisher.representation().string_representation(rep.xml);
    object_variant.publisher(publisher);
}

template<>
void fill_object_variant<Agent::SUBSCRIBER_OBJK>(
        uint16_t participant_id,
        XmlRep rep,
        dds::xrce::ObjectVariant& object_variant)
{
    dds::xrce::OBJK_SUBSCRIBER_Representation subscriber;
    subscriber.participant_id(convertion::raw_to_objectid(participant_id));
    subscriber.representation().string_representation(rep.xml);
    object_variant.subscriber(subscriber);
}

template<>
void fill_object_variant<Agent::DATAWRITER_OBJK>(
        uint16_t publisher_id,
        XmlRep rep,
        dds::xrce::ObjectVariant& object_variant)
{
    dds::xrce::DATAWRITER_Representation datawriter;
    datawriter.publisher_id(convertion::raw_to_objectid(publisher_id));
    datawriter.representation().xml_string_representation(rep.xml);
    object_variant.data_writer(datawriter);
}

template<>
void fill_object_variant<Agent::DATAWRITER_OBJK>(
        uint16_t publisher_id,
        RefRep rep,
        dds::xrce::ObjectVariant& object_variant)
{
    dds::xrce::DATAWRITER_Representation datawriter;
    datawriter.publisher_id(convertion::raw_to_objectid(publisher_id));
    datawriter.representation().object_reference(rep.ref);
    object_variant.data_writer(datawriter);
}

template<>
void fill_object_variant<Agent::DATAREADER_OBJK>(
        uint16_t subscriber_id,
        XmlRep rep,
        dds::xrce::ObjectVariant& object_variant)
{
    dds::xrce::DATAREADER_Representation datareader;
    datareader.subscriber_id(convertion::raw_to_objectid(subscriber_id));
    datareader.representation().xml_string_representation(rep.xml);
    object_variant.data_reader(datareader);
}

template<>
void fill_object_variant<Agent::DATAREADER_OBJK>(
        uint16_t subscriber_id,
        RefRep rep,
        dds::xrce::ObjectVariant& object_variant)
{
    dds::xrce::DATAREADER_Representation datareader;
    datareader.subscriber_id(convertion::raw_to_objectid(subscriber_id));
    datareader.representation().object_reference(rep.ref);
    object_variant.data_reader(datareader);
}

template<Agent::ObjectKind object_kind, typename U, typename T>
bool create_object(
        uint32_t client_key,
        uint16_t raw_id,
        T parent_id,
        U rep,
        uint8_t flag,
        Agent::OpResult& op_result)
{
    bool rv = false;

    if (std::shared_ptr<ProxyClient> client = Root::instance().get_client(raw_to_clientkey(client_key)))
    {
        if (object_kind == (raw_id & 0x000F))
        {
            dds::xrce::CreationMode creation_mode{};
            creation_mode.reuse(0 != (flag & Agent::REUSE_MODE));
            creation_mode.replace(0 != (flag & Agent::REPLACE_MODE));

            dds::xrce::ObjectVariant object_variant;
            fill_object_variant<object_kind, U, T>(parent_id, rep, object_variant);

            dds::xrce::ObjectId object_id = convertion::raw_to_objectid(raw_id);
            dds::xrce::ResultStatus result = client->create(creation_mode, object_id, object_variant);
            op_result = Agent::OpResult(result.status());
            rv = (dds::xrce::STATUS_OK == result.status() || dds::xrce::STATUS_OK_MATCHED == result.status());
        }
        else
        {
            op_result = Agent::OpResult(dds::xrce::STATUS_ERR_INVALID_DATA);
        }
    }
    else
    {
        op_result = Agent::OpResult(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
    }

    return rv;
}

} // unnamed namespace

/**********************************************************************************************************************
 * Client.
 **********************************************************************************************************************/
bool Agent::create_client(
        uint32_t key,
        uint8_t session,
        uint16_t mtu,
        OpResult& op_result)
{
    Root& root = Root::instance();
    dds::xrce::CLIENT_Representation client_representation;
    dds::xrce::AGENT_Representation agent_representation;
    dds::xrce::ResultStatus result;

    client_representation.client_key(convertion::raw_to_clientkey(key));
    client_representation.xrce_cookie(dds::xrce::XRCE_COOKIE);
    client_representation.xrce_version(dds::xrce::XRCE_VERSION);
    client_representation.session_id(session);
    client_representation.mtu(mtu);
    result = root.create_client(client_representation, agent_representation);
    op_result = OpResult(result.status());

    return (dds::xrce::STATUS_OK == result.status());
}

bool Agent::delete_client(
        uint32_t key,
        OpResult& op_result)
{
    Root& root = Root::instance();
    dds::xrce::ResultStatus result = root.delete_client(convertion::raw_to_clientkey(key));

    if (dds::xrce::STATUS_OK != result.status())
    {
        op_result = OpResult(result.status());
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
        OpResult& op_result)
{
    return create_object<Agent::PARTICIPANT_OBJK>
            (client_key, participant_id, domain_id, RefRep{ref}, flag, op_result);
}

bool Agent::create_participant_by_xml(
        uint32_t client_key,
        uint16_t participant_id,
        int16_t domain_id,
        const char* xml,
        uint8_t flag,
        OpResult& op_result)
{
    return create_object<Agent::PARTICIPANT_OBJK>
            (client_key, participant_id, domain_id, XmlRep{xml}, flag, op_result);
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
        OpResult& op_result)
{
    return create_object<Agent::TOPIC_OBJK>
            (client_key, topic_id, participant_id, RefRep{ref}, flag, op_result);
}

bool Agent::create_topic_by_xml(
        uint32_t client_key,
        uint16_t topic_id,
        uint16_t participant_id,
        const char* xml,
        uint8_t flag,
        OpResult& op_result)
{
    return create_object<Agent::TOPIC_OBJK>
            (client_key, topic_id, participant_id, XmlRep{xml}, flag, op_result);
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
        OpResult& op_result)
{
    return create_object<Agent::PUBLISHER_OBJK>
            (client_key, publisher_id, participant_id, XmlRep{xml}, flag, op_result);
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
        OpResult& op_result)
{
    return create_object<Agent::SUBSCRIBER_OBJK>
            (client_key, subscriber_id, participant_id, XmlRep{xml}, flag, op_result);
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
        OpResult& op_result)
{
    return create_object<Agent::DATAWRITER_OBJK>
            (client_key, datawriter_id, publisher_id, RefRep{ref}, flag, op_result);
}

bool Agent::create_datawriter_by_xml(
        uint32_t client_key,
        uint16_t datawriter_id,
        uint16_t publisher_id,
        const char* xml,
        uint8_t flag,
        OpResult& op_result)
{
    return create_object<Agent::DATAWRITER_OBJK>
            (client_key, datawriter_id, publisher_id, XmlRep{xml}, flag, op_result);
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
        OpResult& op_result)
{
    return create_object<Agent::DATAREADER_OBJK>
            (client_key, datareader_id, subscriber_id, RefRep{ref}, flag, op_result);
}

bool Agent::create_datareader_by_xml(
        uint32_t client_key,
        uint16_t datareader_id,
        uint16_t subscriber_id,
        const char* xml,
        uint8_t flag,
        OpResult& op_result)
{
    return create_object<Agent::DATAREADER_OBJK>
            (client_key, datareader_id, subscriber_id, XmlRep{xml}, flag, op_result);
}

/**********************************************************************************************************************
 * Delete Object.
 **********************************************************************************************************************/
bool Agent::delete_object(
        uint32_t client_key,
        uint16_t object_id,
        OpResult& op_result)
{
    bool rv = false;
    Root& root = Root::instance();

    if (std::shared_ptr<ProxyClient> client = root.get_client(convertion::raw_to_clientkey(client_key)))
    {
        dds::xrce::ResultStatus result = client->delete_object(convertion::raw_to_objectid(object_id));
        op_result = OpResult(result.status());
        rv = (dds::xrce::STATUS_OK == result.status() || dds::xrce::STATUS_OK_MATCHED == result.status());
    }
    else
    {
        op_result = OpResult(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
    }

    return rv;
}

/**********************************************************************************************************************
 * Config.
 **********************************************************************************************************************/
bool Agent::load_config_file(const std::string& file_path)
{
    return Root::instance().load_config_file(file_path);
}

/**********************************************************************************************************************
 * Write Data.
 **********************************************************************************************************************/
bool Agent::write(
        uint32_t client_key,
        uint16_t datawriter_id,
        uint8_t* buf,
        size_t len,
        OpResult& op_result)
{
    bool rv = false;
    Root& root = Root::instance();

    if (std::shared_ptr<ProxyClient> client = root.get_client(convertion::raw_to_clientkey(client_key)))
    {
         DataWriter* datawriter = dynamic_cast<DataWriter*>(client->get_object(convertion::raw_to_objectid(datawriter_id)));
         if (datawriter)
         {
             std::vector<uint8_t> data(buf, buf + len);
             rv = datawriter->write(data);
             op_result = rv ? OpResult::OK : OpResult::WRITE_ERROR;
         }
        else
        {
            op_result = OpResult::UNKNOWN_REFERENCE_ERROR;
        }
    }
    else
    {
        op_result = OpResult::UNKNOWN_REFERENCE_ERROR;
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
