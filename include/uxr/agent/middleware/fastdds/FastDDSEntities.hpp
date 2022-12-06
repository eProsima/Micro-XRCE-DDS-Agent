// Copyright 2017-present Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef UXR__AGENT__MIDDLEWARE__FASTDDS__ENTITIES_HPP_
#define UXR__AGENT__MIDDLEWARE__FASTDDS__ENTITIES_HPP_

//#include <dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastrtps/attributes/all_attributes.h>
#include <uxr/agent/types/TopicPubSubType.hpp>
#include <uxr/agent/types/XRCETypes.hpp>

#include <unordered_map>

namespace eprosima {
namespace uxr {

class FastDDSType;
class FastDDSTopic;


/**********************************************************************************************************************
 * FastDDSParticipant
 **********************************************************************************************************************/
class FastDDSParticipant
{
public:
    FastDDSParticipant(int16_t domain_id)
        : ptr_{nullptr}
        , factory_{fastdds::dds::DomainParticipantFactory::get_instance()}
        , domain_id_{domain_id}
    {
        factory_->load_profiles();
    }

    ~FastDDSParticipant();

    bool create_by_ref(const std::string& ref);
    bool create_by_xml(const std::string& xml);
    bool create_by_bin(const dds::xrce::OBJK_DomainParticipant_Binary& participant_xrce);
    bool match_from_ref(const std::string& ref) const;
    bool match_from_xml(const std::string& xml) const;
    bool match_from_bin(const dds::xrce::OBJK_DomainParticipant_Binary& participant_xrce) const;

    // Proxy methods

    int16_t domain_id() const { return domain_id_; }
    fastrtps::rtps::GUID_t guid() const { return ptr_->guid(); }
    fastdds::dds::DomainParticipant* get_ptr() const { return ptr_; }

    ReturnCode_t unregister_type(
            const std::string& typeName);

    fastdds::dds::Topic* create_topic(
        const std::string& topic_name,
        const std::string& type_name,
        const fastdds::dds::TopicQos& qos,
        fastdds::dds::TopicListener* listener = nullptr,
        const fastdds::dds::StatusMask& mask = fastdds::dds::StatusMask::all());

    ReturnCode_t delete_topic(
        fastdds::dds::Topic* topic);

    fastdds::dds::Publisher* create_publisher(
            const fastdds::dds::PublisherQos& qos,
            fastdds::dds::PublisherListener* listener = nullptr,
            const fastdds::dds::StatusMask& mask = fastdds::dds::StatusMask::all());

    ReturnCode_t delete_publisher(
        fastdds::dds::Publisher* publisher);

    fastdds::dds::Subscriber* create_subscriber(
            const fastdds::dds::SubscriberQos& qos,
            fastdds::dds::SubscriberListener* listener = nullptr,
            const fastdds::dds::StatusMask& mask = fastdds::dds::StatusMask::all());

    ReturnCode_t delete_subscriber(
        fastdds::dds::Subscriber* subscriber);

    // Types and topics registration

    bool register_local_type(
            const std::shared_ptr<FastDDSType>& type);

    bool unregister_local_type(
            const std::string& type_name);

    std::shared_ptr<FastDDSType> find_local_type(
            const std::string& type_name) const;

    bool register_local_topic(
            const std::shared_ptr<FastDDSTopic>& topic);

    bool unregister_local_topic(
            const std::string& topic_name);

    std::shared_ptr<FastDDSTopic> find_local_topic(
            const std::string& topic_name) const;

    fastdds::dds::DomainParticipant* operator * ();

    const fastdds::dds::DomainParticipant* operator * () const;

private:
    fastdds::dds::DomainParticipant* ptr_;
    fastdds::dds::DomainParticipantFactory* factory_;
    int16_t domain_id_;
    std::unordered_map<std::string, std::weak_ptr<FastDDSType>> type_register_;
    std::unordered_map<std::string, std::weak_ptr<FastDDSTopic>> topic_register_;
};

/**********************************************************************************************************************
 * FastDDSTopic
 **********************************************************************************************************************/
class FastDDSType
{
public:
    FastDDSType(
            const fastdds::dds::TypeSupport& type_support,
            const std::shared_ptr<FastDDSParticipant>& participant)
        : type_support_{type_support}
        , participant_{participant}
    {}

    ~FastDDSType();

    fastdds::dds::TypeSupport& get_type_support() { return type_support_; }
    const fastdds::dds::TypeSupport& get_type_support() const { return type_support_; }

private:
    fastdds::dds::TypeSupport type_support_;
    std::shared_ptr<FastDDSParticipant> participant_;
};

class FastDDSTopic
{
public:
    FastDDSTopic(const std::shared_ptr<FastDDSParticipant>& participant)
        : participant_(participant)
        , type_{nullptr}
        , ptr_{nullptr}
    {}

    ~FastDDSTopic();

    bool create_by_ref(const std::string& ref);
    bool create_by_xml(const std::string& xml);
    bool create_by_name_type(const std::string& name,
        const std::shared_ptr<FastDDSType>& type);
    bool match_from_ref(const std::string& ref) const;
    bool match_from_xml(const std::string& xml) const;
    bool match_from_bin(const dds::xrce::OBJK_Topic_Binary& topic_xrce) const;
    bool match(const fastrtps::TopicAttributes& attrs) const;

    const std::string& get_name() const { return ptr_->get_name(); }
    const std::shared_ptr<FastDDSType>& get_type() const { return type_; }
    fastdds::dds::Topic* get_ptr() const { return ptr_; }

private:
    bool create_by_attributes(const fastrtps::TopicAttributes& attrs);

private:
    std::shared_ptr<FastDDSParticipant> participant_;
    std::shared_ptr<FastDDSType> type_;
    fastdds::dds::Topic* ptr_;
};


/**********************************************************************************************************************
 * FastDDSPublisher
 **********************************************************************************************************************/
class FastDDSPublisher
{
public:
    FastDDSPublisher(const std::shared_ptr<FastDDSParticipant>& participant)
        : participant_{participant}
        , ptr_{nullptr}
    {}

    ~FastDDSPublisher();

    bool create_by_xml(const std::string& xml);
    bool create_by_bin(const dds::xrce::OBJK_Publisher_Binary&  publisher_xrce);

    fastdds::dds::DataWriter* create_datawriter(
            fastdds::dds::Topic* topic,
            const fastdds::dds::DataWriterQos& qos = fastdds::dds::DATAWRITER_QOS_DEFAULT,
            fastdds::dds::DataWriterListener* listener = nullptr,
            const fastdds::dds::StatusMask& mask = fastdds::dds::StatusMask::all());

    ReturnCode_t delete_datawriter(
        fastdds::dds::DataWriter* writer);

    std::shared_ptr<FastDDSParticipant> get_participant() const { return participant_; }

private:
    std::shared_ptr<FastDDSParticipant> participant_;
    fastdds::dds::Publisher* ptr_;
};


/**********************************************************************************************************************
 * FastDDSSubscriber
 **********************************************************************************************************************/
class FastDDSSubscriber
{
public:
    FastDDSSubscriber(const std::shared_ptr<FastDDSParticipant>& participant)
        : participant_{participant}
        , ptr_{nullptr}
    {}

    ~FastDDSSubscriber();

    bool create_by_xml(const std::string& xml);
    bool create_by_bin(const dds::xrce::OBJK_Subscriber_Binary&  subscriber_xrce);

    fastdds::dds::DataReader* create_datareader(
            fastdds::dds::TopicDescription* topic,
            const fastdds::dds::DataReaderQos& reader_qos,
            fastdds::dds::DataReaderListener* listener = nullptr,
            const fastdds::dds::StatusMask& mask = fastdds::dds::StatusMask::all());

    ReturnCode_t delete_datareader(
        fastdds::dds::DataReader* reader);


    std::shared_ptr<FastDDSParticipant> get_participant() const { return participant_; }

private:
    std::shared_ptr<FastDDSParticipant> participant_;
    fastdds::dds::Subscriber* ptr_;

};

/**********************************************************************************************************************
 * FastDDSDataWriter
 **********************************************************************************************************************/
class FastDDSDataWriter
{
public:
    FastDDSDataWriter(const std::shared_ptr<FastDDSPublisher>& publisher)
        : publisher_{publisher}
        , topic_{nullptr}
        , ptr_{nullptr}
    {}

    ~FastDDSDataWriter();

    fastrtps::rtps::GUID_t guid() const { return ptr_->guid(); }

    bool create_by_ref(const std::string& ref);
    bool create_by_xml(const std::string& xml);
    bool create_by_bin(
        const dds::xrce::OBJK_DataWriter_Binary& datawriter_xrce,
        std::shared_ptr<eprosima::uxr::FastDDSTopic> topic);
    bool match(const fastrtps::PublisherAttributes& attrs) const;
    bool match_from_bin(const dds::xrce::OBJK_DataWriter_Binary& datawriter_xrce) const;
    bool write(const std::vector<uint8_t>& data);
    const fastdds::dds::DataWriter* ptr() const;
    const fastdds::dds::DomainParticipant* participant() const;

private:
    std::shared_ptr<FastDDSPublisher> publisher_;
    std::shared_ptr<FastDDSTopic> topic_;
    fastdds::dds::DataWriter* ptr_;
};

/**********************************************************************************************************************
 * FastDataReader
 **********************************************************************************************************************/
class FastDDSDataReader
{
public:
    FastDDSDataReader(const std::shared_ptr<FastDDSSubscriber>& subscriber)
        : subscriber_{subscriber}
        , ptr_{nullptr}
    {}

    ~FastDDSDataReader();

    fastrtps::rtps::GUID_t guid() const { return ptr_->guid(); }

    bool create_by_ref(const std::string& ref);
    bool create_by_xml(const std::string& xml);
    bool create_by_bin(
        const dds::xrce::OBJK_DataReader_Binary& datawriter_xrce,
        std::shared_ptr<eprosima::uxr::FastDDSTopic> topic);
    bool match_from_ref(const std::string& ref) const;
    bool match_from_xml(const std::string& xml) const;
    bool match_from_bin(const dds::xrce::OBJK_DataReader_Binary& datawriter_xrce) const;
    bool read(
            std::vector<uint8_t>& data,
            std::chrono::milliseconds timeout,
            fastdds::dds::SampleInfo& sample_info);
    const fastdds::dds::DataReader* ptr() const;
    const fastdds::dds::DomainParticipant* participant() const;

private:
    std::shared_ptr<FastDDSSubscriber> subscriber_;
    std::shared_ptr<FastDDSTopic> topic_;
    fastdds::dds::DataReader* ptr_;
};

/**********************************************************************************************************************
 * FastRequester
 **********************************************************************************************************************/
class FastDDSRequester
{
public:
    FastDDSRequester(
            const std::shared_ptr<FastDDSParticipant>& participant,
            const std::shared_ptr<FastDDSTopic>& request_topic,
            const std::shared_ptr<FastDDSTopic>& reply_topic)
        : participant_{participant}
        , request_topic_{request_topic}
        , reply_topic_{reply_topic}
        , publisher_ptr_{nullptr}
        , subscriber_ptr_{nullptr}
        , publisher_id_{}
        , sequence_to_sequence_{}
    {}

    ~FastDDSRequester();

    fastrtps::rtps::GUID_t guid_datareader() const { return datareader_ptr_->guid(); }
    fastrtps::rtps::GUID_t guid_datawriter() const { return datawriter_ptr_->guid(); }

    bool create_by_attributes(
        const fastrtps::RequesterAttributes& attrs);
    bool match_from_ref(const std::string& ref) const;
    bool match_from_xml(const std::string& xml) const;
    bool match_from_bin(const dds::xrce::OBJK_Requester_Binary& requester_xrce) const;

    bool write(
        uint32_t sequence_number,
        const std::vector<uint8_t>& data);

    bool read(
        uint32_t& sequence_number,
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout,
        fastdds::dds::SampleInfo& info);

    const fastdds::dds::DomainParticipant* get_participant() const;

    const fastdds::dds::DataWriter* get_request_datawriter() const;

    const fastdds::dds::DataReader* get_reply_datareader() const;

private:
    bool match(const fastrtps::RequesterAttributes& attrs) const;

private:
    std::shared_ptr<FastDDSParticipant> participant_;

    std::shared_ptr<FastDDSType> request_type_;
    std::shared_ptr<FastDDSType> reply_type_;

    std::shared_ptr<FastDDSTopic> request_topic_;
    std::shared_ptr<FastDDSTopic> reply_topic_;

    fastdds::dds::Publisher* publisher_ptr_;
    fastdds::dds::DataWriter* datawriter_ptr_;

    fastdds::dds::Subscriber* subscriber_ptr_;
    fastdds::dds::DataReader* datareader_ptr_;

    dds::GUID_t publisher_id_;
    std::map<int64_t, uint32_t> sequence_to_sequence_;
};

/**********************************************************************************************************************
 * FastReplier
 **********************************************************************************************************************/
class FastDDSReplier
{
public:
    FastDDSReplier(
            const std::shared_ptr<FastDDSParticipant>& participant,
            const std::shared_ptr<FastDDSTopic>& request_topic,
            const std::shared_ptr<FastDDSTopic>& reply_topic)
        : participant_{participant}
        , request_topic_{request_topic}
        , reply_topic_{reply_topic}
        , publisher_ptr_{nullptr}
        , subscriber_ptr_{nullptr}
    {}

    ~FastDDSReplier();

    fastrtps::rtps::GUID_t guid_datareader() const { return datareader_ptr_->guid(); }
    fastrtps::rtps::GUID_t guid_datawriter() const { return datawriter_ptr_->guid(); }

    bool create_by_attributes(
        const fastrtps::ReplierAttributes& attrs);
    bool match_from_ref(const std::string& ref) const;
    bool match_from_xml(const std::string& xml) const;
    bool match_from_bin(const dds::xrce::OBJK_Replier_Binary& replier_xrce) const;

    bool write(const std::vector<uint8_t>& data);
    bool read(std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout,
        fastdds::dds::SampleInfo& info);

    const fastdds::dds::DomainParticipant* get_participant() const;

    const fastdds::dds::DataReader* get_request_datareader() const;

    const fastdds::dds::DataWriter* get_reply_datawriter() const;

private:
    bool match(const fastrtps::ReplierAttributes& attrs) const;
    void transform_sample_identity(
        const fastrtps::rtps::SampleIdentity& fast_identity,
        dds::SampleIdentity& dds_identity);
    void transport_sample_identity(
        const dds::SampleIdentity& dds_identity,
        fastrtps::rtps::SampleIdentity& fast_identity);

private:
    std::shared_ptr<FastDDSParticipant> participant_;

    std::shared_ptr<FastDDSType> request_type_;
    std::shared_ptr<FastDDSType> reply_type_;

    std::shared_ptr<FastDDSTopic> request_topic_;
    std::shared_ptr<FastDDSTopic> reply_topic_;

    fastdds::dds::Publisher* publisher_ptr_;
    fastdds::dds::DataWriter* datawriter_ptr_;

    fastdds::dds::Subscriber* subscriber_ptr_;
    fastdds::dds::DataReader* datareader_ptr_;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR__AGENT__MIDDLEWARE__FASTDDS__ENTITIES_HPP_