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
#include <uxr/agent/types/TopicPubSubType.hpp>

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
private:
    std::shared_ptr<FastDDSParticipant> participant_;
    std::shared_ptr<FastDDSType> type_;
    fastdds::dds::Topic* ptr_;
public:
    uint16_t topic_id;
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
    // TODO.
};

class FastDDSReplier
{
    // TODO.
};

} // namespace uxr
} // namespace eprosima

#endif // UXR__AGENT__MIDDLEWARE__FASTDDS__ENTITIES_HPP_