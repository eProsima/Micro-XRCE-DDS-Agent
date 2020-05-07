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

class FastDDSParticipant : public std::enable_shared_from_this<FastDDSParticipant>
{
public:
    FastDDSParticipant(int16_t domain_id)
        : domain_id_{domain_id}
        , factory_{fastdds::dds::DomainParticipantFactory::get_instance()}
        , ptr_{nullptr}
    {}

    ~FastDDSParticipant();

    bool create_by_ref(const std::string& ref);
    bool create_by_xml(const std::string& xml);
    bool match_from_ref(const std::string& ref) const;
    bool match_from_xml(const std::string& xml) const;
    int16_t get_domain_id() const { return domain_id_; }

    bool register_topic(
            const fastrtps::TopicAttributes& attrs,
            std::shared_ptr<FastDDSType>& type,
            std::shared_ptr<FastDDSTopic> topic);

    bool unregister_topic(
            const std::string& topic_name);

    bool find_topic(
            const std::string& topic_name,
            std::shared_ptr<FastDDSTopic>& topic);

    friend class FastDDSTopic;
    friend class FastDDSPublisher;
    friend class FastDDSSubscriber;
    friend class FastDDSRequester;
    friend class FastDDSReplier;
private:
    int16_t domain_id_;
    fastdds::dds::DomainParticipantFactory* factory_;
    fastdds::dds::DomainParticipant* ptr_;
    std::unordered_map<std::string, std::weak_ptr<FastDDSTopic>> topics_;
    std::unordered_map<std::string, std::weak_ptr<FastDDSType>> type_register_;
};

class FastDDSType : public TopicPubSubType
{
public:
    FastDDSType(const std::shared_ptr<FastDDSParticipant>& participant)
        : TopicPubSubType{false}
        , participant_{participant}
    {}

    ~FastDDSType();

private:
    std::shared_ptr<FastDDSParticipant> participant_;
};

class FastDDSTopic : public std::enable_shared_from_this<FastDDSTopic>
{
public:
    FastDDSTopic(const std::shared_ptr<FastDDSParticipant>& participant)
        : participant_{participant}
        , type_{nullptr}
        , ptr_{nullptr}
    {}

    ~FastDDSTopic();

    bool create_by_ref(
        const std::string& ref,
        uint16_t topic_id);
    bool create_by_xml(
        const std::string& xml,
        uint16_t topic_id);
    bool match_from_ref(const std::string& ref) const;
    bool match_from_xml(const std::string& xml) const;

    friend class FastDDSDataWriter;
    friend class FastDDSDataReader;
    friend class FastDDSRequester;
    friend class FastDDSReplier;
private:
    bool create_by_attributes(
        const fastrtps::TopicAttributes& attrs,
        uint16_t topic_id);

private:
    std::shared_ptr<FastDDSParticipant> participant_;
    std::shared_ptr<FastDDSType> type_;
    fastdds::dds::Topic* ptr_;
public:
    uint16_t topic_id_;
};

class FastDDSPublisher
{
public:
    FastDDSPublisher(const std::shared_ptr<FastDDSParticipant>& participant) 
        : participant_{participant}
        , ptr_{nullptr}
    {}
    
    ~FastDDSPublisher();

    bool create_by_xml(const std::string& xml);

    friend class FastDDSDataWriter;
private:
    std::shared_ptr<FastDDSParticipant> participant_;
    fastdds::dds::Publisher* ptr_;
};

class FastDDSSubscriber
{
public:
    FastDDSSubscriber(const std::shared_ptr<FastDDSParticipant>& participant) 
        : participant_{participant}
        , ptr_{nullptr}
    {}

    ~FastDDSSubscriber();

    bool create_by_xml(const std::string& xml);
    
    friend class FastDDSDataReader;
private:
    std::shared_ptr<FastDDSParticipant> participant_;
    fastdds::dds::Subscriber* ptr_;

};

class FastDDSDataWriter
{
public:
    FastDDSDataWriter(const std::shared_ptr<FastDDSPublisher>& publisher)
        : publisher_{publisher}
        , ptr_{nullptr}
    {}

    ~FastDDSDataWriter();

    bool create_by_ref(
            const std::string& ref,
            uint16_t& topic_id);
    bool create_by_xml(
            const std::string& xml,
            uint16_t& topic_id);
    bool match_from_ref(const std::string& ref) const;
    bool match_from_xml(const std::string& xml) const;
    bool write(const std::vector<uint8_t>& data);
private:
    std::shared_ptr<FastDDSPublisher> publisher_;
    fastdds::dds::DataWriter* ptr_;
};

class FastDDSDataReader
{
public:
    FastDDSDataReader(const std::shared_ptr<FastDDSSubscriber>& subscriber)
        : subscriber_{subscriber}
        , ptr_{nullptr}
    {}

    ~FastDDSDataReader();

    bool create_by_ref(
            const std::string& ref,
            uint16_t& topic_id);
    bool create_by_xml(
            const std::string& xml,
            uint16_t& topic_id);
    bool match_from_ref(const std::string& ref) const;
    bool match_from_xml(const std::string& xml) const;
    bool read(
            std::vector<uint8_t>& data,
            std::chrono::milliseconds timeout);   
private:
    std::shared_ptr<FastDDSSubscriber> subscriber_;
    fastdds::dds::DataReader* ptr_;
};

class FastDDSRequester
{
public:
    FastDDSRequester(const std::shared_ptr<FastDDSParticipant>& participant) 
        : participant_{participant}
        , request_topic_{nullptr}
        , reply_topic_{nullptr}
        , publisher_ptr_{nullptr}
        , datawriter_ptr_{nullptr}
        , subscriber_ptr_{nullptr}
        , datareader_ptr_{nullptr}
        , publisher_id_{}
        , sequence_to_sequence_{}
    {}

    ~FastDDSRequester();

    bool create_by_ref(const std::string& ref);
    bool create_by_xml(const std::string& xml);
    bool match_from_ref(const std::string& ref) const;
    bool match_from_xml(const std::string& xml) const;

    bool write(
        uint32_t sequence_number,
        const std::vector<uint8_t>& data);

    bool read(
        uint32_t& sequence_number,
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout);

private:
    bool match(const fastrtps::RequesterAttributes& attrs) const;
    bool create_by_attributes(
        const fastrtps::RequesterAttributes& attrs);
    
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

class FastDDSReplier
{
public:
    FastDDSReplier(const std::shared_ptr<FastDDSParticipant>& participant) 
        : participant_{participant}
        , request_topic_{nullptr}
        , reply_topic_{nullptr}
        , publisher_ptr_{nullptr}
        , datawriter_ptr_{nullptr}
        , subscriber_ptr_{nullptr}
        , datareader_ptr_{nullptr}
    {}

    ~FastDDSReplier();

    bool create_by_ref(const std::string& ref);
    bool create_by_xml(const std::string& xml);
    bool match_from_ref(const std::string& ref) const;
    bool match_from_xml(const std::string& xml) const;

    bool write(const std::vector<uint8_t>& data);
    bool read(std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout);

private:
    bool match(const fastrtps::ReplierAttributes& attrs) const;
    bool create_by_attributes(
        const fastrtps::ReplierAttributes& attrs);
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