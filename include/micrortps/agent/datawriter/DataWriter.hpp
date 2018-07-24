// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _MICRORTPS_AGENT_DATAWRITER_DATAWRITER_HPP_
#define _MICRORTPS_AGENT_DATAWRITER_DATAWRITER_HPP_

#include <micrortps/agent/object/XRCEObject.hpp>
#include <micrortps/agent/types/TopicPubSubType.hpp>
#include <fastrtps/publisher/PublisherListener.h>
#include <string>
#include <set>

namespace eprosima {

namespace fastrtps {
class Participant;
class Publisher;
} // namespace fastrtps

namespace micrortps {

class Publisher;
class Topic;
class WRITE_DATA_Payload;

class DataWriter : public XRCEObject, public fastrtps::PublisherListener
{
public:
    DataWriter(const dds::xrce::ObjectId& object_id,
               const std::shared_ptr<Publisher>& publisher,
               const std::string& profile_name = "");
    ~DataWriter() override;

    DataWriter(DataWriter&&)      = delete;
    DataWriter(const DataWriter&) = delete;
    DataWriter& operator=(DataWriter&&) = delete;
    DataWriter& operator=(const DataWriter&) = delete;

    bool init(const ObjectContainer& root_objects);
    bool init(const std::string& xml_rep, const ObjectContainer& root_objects);
    const dds::xrce::ResultStatus& write(dds::xrce::DataRepresentation& data);
    bool write(dds::xrce::WRITE_DATA_Payload_Data& write_data);
    void release(ObjectContainer&) override {}

private:
    void onPublicationMatched(fastrtps::Publisher* pub, fastrtps::rtps::MatchingInfo& info) override;
    std::shared_ptr<Publisher> publisher_;
    std::shared_ptr<Topic> topic_;
    fastrtps::Publisher* rtps_publisher_;
    std::string rtps_publisher_prof_;
    TopicPubSubType topic_type_;
    dds::xrce::ResultStatus result_status_;
    std::set<dds::xrce::ObjectId> objects_;
};

} // namespace micrortps
} // namespace eprosima

#endif //_MICRORTPS_AGENT_DATAWRITER_DATAWRITER_HPP_
