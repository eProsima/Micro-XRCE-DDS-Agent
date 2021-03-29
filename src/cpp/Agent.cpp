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
#include <uxr/agent/utils/Conversion.hpp>
#include <uxr/agent/datawriter/DataWriter.hpp>
#include <uxr/agent/middleware/utils/Callbacks.hpp>

namespace eprosima {
namespace uxr {

namespace {

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
    topic.participant_id(conversion::raw_to_objectid(participant_id, Agent::PARTICIPANT_OBJK));
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
    topic.participant_id(conversion::raw_to_objectid(participant_id, Agent::PARTICIPANT_OBJK));
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
    publisher.participant_id(conversion::raw_to_objectid(participant_id, Agent::PARTICIPANT_OBJK));
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
    subscriber.participant_id(conversion::raw_to_objectid(participant_id, Agent::PARTICIPANT_OBJK));
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
    datawriter.publisher_id(conversion::raw_to_objectid(publisher_id, Agent::PUBLISHER_OBJK));
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
    datawriter.publisher_id(conversion::raw_to_objectid(publisher_id, Agent::PUBLISHER_OBJK));
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
    datareader.subscriber_id(conversion::raw_to_objectid(subscriber_id, Agent::SUBSCRIBER_OBJK));
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
    datareader.subscriber_id(conversion::raw_to_objectid(subscriber_id, Agent::SUBSCRIBER_OBJK));
    datareader.representation().object_reference(rep.ref);
    object_variant.data_reader(datareader);
}

template<>
void fill_object_variant<Agent::REQUESTER_OBJK>(
        uint16_t participant_id,
        RefRep rep,
        dds::xrce::ObjectVariant& object_variant)
{
    dds::xrce::REQUESTER_Representation requester;
    requester.participant_id(conversion::raw_to_objectid(participant_id, Agent::PARTICIPANT_OBJK));
    requester.representation().object_reference(rep.ref);
    object_variant.requester(requester);
}

template<>
void fill_object_variant<Agent::REQUESTER_OBJK>(
        uint16_t participant_id,
        XmlRep rep,
        dds::xrce::ObjectVariant& object_variant)
{
    dds::xrce::REQUESTER_Representation requester;
    requester.participant_id(conversion::raw_to_objectid(participant_id, Agent::PARTICIPANT_OBJK));
    requester.representation().xml_string_representation(rep.xml);
    object_variant.requester(requester);
}

template<>
void fill_object_variant<Agent::REPLIER_OBJK>(
        uint16_t participant_id,
        RefRep rep,
        dds::xrce::ObjectVariant& object_variant)
{
    dds::xrce::REPLIER_Representation replier;
    replier.participant_id(conversion::raw_to_objectid(participant_id, Agent::PARTICIPANT_OBJK));
    replier.representation().object_reference(rep.ref);
    object_variant.replier(replier);
}

template<>
void fill_object_variant<Agent::REPLIER_OBJK>(
        uint16_t participant_id,
        XmlRep rep,
        dds::xrce::ObjectVariant& object_variant)
{
    dds::xrce::REPLIER_Representation replier;
    replier.participant_id(conversion::raw_to_objectid(participant_id, Agent::PARTICIPANT_OBJK));
    replier.representation().xml_string_representation(rep.xml);
    object_variant.replier(replier);
}

} // unnamed namespace

/**********************************************************************************************************************
 * Des/Constructor.
 **********************************************************************************************************************/
Agent::Agent()
    : root_(new Root())
    , callback_factory_(callback_factory_.getInstance())
{}

Agent::~Agent() = default;

/**********************************************************************************************************************
 * Client.
 **********************************************************************************************************************/
bool Agent::create_client(
        uint32_t key,
        uint8_t session,
        uint16_t mtu,
        Middleware::Kind middleware_kind,
        OpResult& op_result)
{
    dds::xrce::CLIENT_Representation client_representation;
    dds::xrce::AGENT_Representation agent_representation;
    dds::xrce::ResultStatus result;

    client_representation.client_key(conversion::raw_to_clientkey(key));
    client_representation.xrce_cookie(dds::xrce::XRCE_COOKIE);
    client_representation.xrce_version(dds::xrce::XRCE_VERSION);
    client_representation.session_id(session);
    client_representation.mtu(mtu);
    result = root_->create_client(client_representation, agent_representation, middleware_kind);
    op_result = OpResult(result.status());

    return (dds::xrce::STATUS_OK == result.status());
}

bool Agent::delete_client(
        uint32_t key,
        OpResult& op_result)
{
    dds::xrce::ResultStatus result = root_->delete_client(conversion::raw_to_clientkey(key));

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

bool Agent::delete_participant(
        uint32_t client_key,
        uint16_t participant_id,
        OpResult& op_result)
{
    return delete_object<Agent::PARTICIPANT_OBJK>
            (client_key, participant_id, op_result);
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

bool Agent::delete_topic(
        uint32_t client_key,
        uint16_t topic_id,
        OpResult& op_result)
{
    return delete_object<Agent::TOPIC_OBJK>
            (client_key, topic_id, op_result);
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

bool Agent::delete_publisher(
        uint32_t client_key,
        uint16_t publisher_id,
        OpResult& op_result)
{
    return delete_object<Agent::PUBLISHER_OBJK>
            (client_key, publisher_id, op_result);
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

bool Agent::delete_subscriber(
        uint32_t client_key,
        uint16_t subscriber_id,
        OpResult& op_result)
{
    return delete_object<Agent::SUBSCRIBER_OBJK>
            (client_key, subscriber_id, op_result);
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

bool Agent::delete_datawriter(
        uint32_t client_key,
        uint16_t datawriter_id,
        OpResult& op_result)
{
    return delete_object<Agent::DATAWRITER_OBJK>
            (client_key, datawriter_id, op_result);
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

bool Agent::delete_datareader(
        uint32_t client_key,
        uint16_t datareader_id,
        OpResult& op_result)
{
    return delete_object<Agent::DATAREADER_OBJK>
            (client_key, datareader_id, op_result);
}

/**********************************************************************************************************************
 * Requester.
 **********************************************************************************************************************/
bool Agent::create_requester_by_ref(
        uint32_t client_key,
        uint16_t requester_id,
        uint16_t participant_id,
        const char* ref,
        uint8_t flag,
        OpResult& op_result)
{
    return create_object<Agent::REQUESTER_OBJK>
            (client_key, requester_id, participant_id, RefRep{ref}, flag, op_result);
}

bool Agent::create_requester_by_xml(
        uint32_t client_key,
        uint16_t requester_id,
        uint16_t participant_id,
        const char* xml,
        uint8_t flag,
        OpResult& op_result)
{
    return create_object<Agent::REQUESTER_OBJK>
            (client_key, requester_id, participant_id, XmlRep{xml}, flag, op_result);
}

bool Agent::delete_requester(
        uint32_t client_key,
        uint16_t requester_id,
        OpResult& op_result)
{
    return delete_object<Agent::REQUESTER_OBJK>
            (client_key, requester_id, op_result);
}

/**********************************************************************************************************************
 * Replier.
 **********************************************************************************************************************/
bool Agent::create_replier_by_ref(
        uint32_t client_key,
        uint16_t replier_id,
        uint16_t participant_id,
        const char* ref,
        uint8_t flag,
        OpResult& op_result)
{
    return create_object<Agent::REPLIER_OBJK>
            (client_key, replier_id, participant_id, RefRep{ref}, flag, op_result);
}

bool Agent::create_replier_by_xml(
        uint32_t client_key,
        uint16_t replier_id,
        uint16_t participant_id,
        const char* xml,
        uint8_t flag,
        OpResult& op_result)
{
    return create_object<Agent::REPLIER_OBJK>
            (client_key, replier_id, participant_id, XmlRep{xml}, flag, op_result);
}

bool Agent::delete_replier(
        uint32_t client_key,
        uint16_t replier_id,
        OpResult& op_result)
{
    return delete_object<Agent::REPLIER_OBJK>
            (client_key, replier_id, op_result);
}

/**********************************************************************************************************************
 * Config.
 **********************************************************************************************************************/
bool Agent::load_config_file(const std::string& file_path)
{
    return root_->load_config_file(file_path);
}

void Agent::set_verbose_level(uint8_t verbose_level)
{
    root_->set_verbose_level(verbose_level);
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

    if (std::shared_ptr<ProxyClient> client = root_->get_client(conversion::raw_to_clientkey(client_key)))
    {
        dds::xrce::ObjectId object_id = conversion::raw_to_objectid(datawriter_id, dds::xrce::OBJK_DATAWRITER);
        std::shared_ptr<DataWriter> datawriter = std::dynamic_pointer_cast<DataWriter>(client->get_object(object_id));
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
    root_->reset();
}

/**********************************************************************************************************************
 * Create Object.
 **********************************************************************************************************************/
template<Agent::ObjectKind object_kind, typename U, typename T>
bool Agent::create_object(
        uint32_t client_key,
        uint16_t raw_id,
        T parent_id,
        U rep,
        uint8_t flag,
        Agent::OpResult& op_result)
{
    bool rv = false;

    if (std::shared_ptr<ProxyClient> client = root_->get_client(conversion::raw_to_clientkey(client_key)))
    {
        dds::xrce::CreationMode creation_mode{};
        creation_mode.reuse(0 != (flag & Agent::REUSE_MODE));
        creation_mode.replace(0 != (flag & Agent::REPLACE_MODE));

        dds::xrce::ObjectVariant object_variant;
        fill_object_variant<object_kind, U, T>(parent_id, rep, object_variant);

        dds::xrce::ObjectId object_id = conversion::raw_to_objectprefix(raw_id);
        dds::xrce::ResultStatus result = client->create_object(creation_mode, object_id, object_variant);
        op_result = Agent::OpResult(result.status());
        rv = (dds::xrce::STATUS_OK == result.status() || dds::xrce::STATUS_OK_MATCHED == result.status());
    }
    else
    {
        op_result = Agent::OpResult(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
    }

    return rv;
}

/**********************************************************************************************************************
 * Delete Object.
 **********************************************************************************************************************/
template<Agent::ObjectKind object_kind>
bool Agent::delete_object(
        uint32_t client_key,
        uint16_t raw_id,
        Agent::OpResult& op_result)
{
    bool rv = false;

    if (std::shared_ptr<ProxyClient> client = root_->get_client(conversion::raw_to_clientkey(client_key)))
    {
        dds::xrce::ResultStatus result = client->delete_object(conversion::raw_to_objectid(raw_id, object_kind));
        op_result = Agent::OpResult(result.status());
        rv = dds::xrce::STATUS_OK == result.status();
    }
    else
    {
        op_result = Agent::OpResult(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
    }

    return rv;
}

template <typename ... Args>
void Agent::add_middleware_callback(
        const Middleware::Kind& middleware_kind,
        const middleware::CallbackKind& callback_kind,
        std::function<void (Args ...)>&& callback_function)
{
    callback_factory_.add_callback(middleware_kind, callback_kind, std::move(callback_function));
}

// Specific template specializations for used callback signatures.
#define AGENT_ADD_MW_CB(...) \
template \
UXR_AGENT_EXPORT void Agent::add_middleware_callback<__VA_ARGS__>( \
    const Middleware::Kind &, \
    const middleware::CallbackKind &, \
    std::function<void (__VA_ARGS__)> &&);

#ifdef UAGENT_FAST_PROFILE
AGENT_ADD_MW_CB(
    const eprosima::fastrtps::Participant *)

AGENT_ADD_MW_CB(
    const eprosima::fastrtps::Participant *,
    const eprosima::fastrtps::Publisher *)

AGENT_ADD_MW_CB(
    const eprosima::fastrtps::Participant *,
    const eprosima::fastrtps::Subscriber *)

AGENT_ADD_MW_CB(
    const eprosima::fastrtps::Participant *,
    const eprosima::fastrtps::Publisher *,
    const eprosima::fastrtps::Subscriber *)

AGENT_ADD_MW_CB(
    const eprosima::fastdds::dds::DomainParticipant *)

AGENT_ADD_MW_CB(
    const eprosima::fastdds::dds::DomainParticipant *,
    const eprosima::fastdds::dds::DataWriter *)

AGENT_ADD_MW_CB(
    const eprosima::fastdds::dds::DomainParticipant *,
    const eprosima::fastdds::dds::DataReader *)

AGENT_ADD_MW_CB(
    const eprosima::fastdds::dds::DomainParticipant *,
    const eprosima::fastdds::dds::DataWriter *,
    const eprosima::fastdds::dds::DataReader *)
#endif  // UAGENT_FAST_PROFILE

} // namespace uxr
} // namespace eprosima
