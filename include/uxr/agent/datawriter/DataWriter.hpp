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

#ifndef _UXR_AGENT_DATAWRITER_DATAWRITER_HPP_
#define _UXR_AGENT_DATAWRITER_DATAWRITER_HPP_

#include <uxr/agent/object/XRCEObject.hpp>
#include <uxr/agent/types/TopicPubSubType.hpp>
#include <fastrtps/publisher/PublisherListener.h>
#include <string>
#include <set>

namespace eprosima {

namespace fastrtps {
class Participant;
class Publisher;
} // namespace fastrtps

namespace uxr {

class Publisher;
class Topic;
class Middleware;

class DataWriter : public XRCEObject, public fastrtps::PublisherListener
{
public:
    DataWriter(const dds::xrce::ObjectId& object_id,
               Middleware* middleware,
               const std::shared_ptr<Publisher>& publisher);
    ~DataWriter() override;

    DataWriter(DataWriter&&) = delete;
    DataWriter(const DataWriter&) = delete;
    DataWriter& operator=(DataWriter&&) = delete;
    DataWriter& operator=(const DataWriter&) = delete;

    bool init_middleware(const dds::xrce::DATAWRITER_Representation& representation,
            const ObjectContainer& root_objects);

    bool write(dds::xrce::WRITE_DATA_Payload_Data& write_data);
    void release(ObjectContainer&) override {}
    bool matched(const dds::xrce::ObjectVariant& new_object_rep) const override;

private:
    void onPublicationMatched(fastrtps::Publisher* pub, fastrtps::rtps::MatchingInfo& info) override;
    std::shared_ptr<Publisher> publisher_;
    std::shared_ptr<Topic> topic_;
    fastrtps::Publisher* rtps_publisher_;
};

} // namespace uxr
} // namespace eprosima

#endif //_UXR_AGENT_DATAWRITER_DATAWRITER_HPP_
