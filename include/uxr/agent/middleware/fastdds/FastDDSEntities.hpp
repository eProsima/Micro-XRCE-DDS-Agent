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

namespace eprosima {
namespace uxr {

class FastDDSParticipant
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

    friend class FastDDSTopic;
private:
    int16_t domain_id_;
    fastdds::dds::DomainParticipantFactory* factory_;
    fastdds::dds::DomainParticipant* ptr_;
};

class FastDDSTopic
{
public:
    FastDDSTopic(const std::shared_ptr<FastDDSParticipant>& participant)
        : participant_{participant}
    {}

    ~FastDDSTopic();

    bool create_by_ref(const std::string& ref);
    bool create_by_xml(const std::string& xml);
    bool match_from_ref(const std::string& ref) const;
    bool match_from_xml(const std::string& xml) const;

private:
    std::shared_ptr<FastDDSParticipant> participant_;
    fastdds::dds::Topic* ptr_;
};

class FastDDSPublisher
{
public:
    FastDDSPublisher(uint16_t participant_id) : participant_id_(participant_id) {}
    ~FastDDSPublisher() = default;

    uint16_t get_participant_id() { return participant_id_; }

private:
    uint16_t participant_id_;
};

class FastDDSSubscriber
{
public:
    FastDDSSubscriber(uint16_t participant_id) : participant_id_(participant_id) {}
    ~FastDDSSubscriber() = default;

    uint16_t get_participant_id() { return participant_id_; }

private:
    uint16_t participant_id_;
};

class FastDDSDataWriter
{
    // TODO.
};

class FastDDSDataReader
{
    // TODO.
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