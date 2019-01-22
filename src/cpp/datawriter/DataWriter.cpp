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

#include <uxr/agent/datawriter/DataWriter.hpp>
#include <uxr/agent/publisher/Publisher.hpp>
#include <uxr/agent/participant/Participant.hpp>
#include <uxr/agent/topic/Topic.hpp>
#include <uxr/agent/middleware/Middleware.hpp>

namespace eprosima {
namespace uxr {

DataWriter::DataWriter(const dds::xrce::ObjectId& object_id,
                       Middleware* middleware,
                       const std::shared_ptr<Publisher>& publisher)
    : XRCEObject(object_id, middleware),
      publisher_(publisher)
{
    publisher_->tie_object(object_id);
}

DataWriter::~DataWriter()
{
    publisher_->untie_object(get_id());
    if (topic_)
    {
        topic_->untie_object(get_id());
    }
    middleware_->delete_datawriter(get_raw_id(), publisher_->get_raw_id());
}

bool DataWriter::init_middleware(
        const dds::xrce::DATAWRITER_Representation& representation,
        const ObjectContainer& root_objects)
{
    bool rv = false;
    switch (representation.representation()._d())
    {
        case dds::xrce::REPRESENTATION_BY_REFERENCE:
        {
            const std::string& ref = representation.representation().object_reference();
            uint16_t topic_id;
            if (middleware_->create_datawriter_from_ref(get_raw_id(), publisher_->get_raw_id(), ref, topic_id))
            {
                dds::xrce::ObjectId topic_xrce_id = {uint8_t(topic_id >> 8), uint8_t(topic_id & 0xFF)};
                topic_ = std::dynamic_pointer_cast<Topic>(root_objects.at(topic_xrce_id));
                topic_->tie_object(get_id());
                rv = true;
            }
            break;
        }
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml = representation.representation().xml_string_representation();
            uint16_t topic_id;
            if (middleware_->create_datawriter_from_xml(get_raw_id(), publisher_->get_raw_id(), xml, topic_id))
            {
                dds::xrce::ObjectId topic_xrce_id = {uint8_t(topic_id >> 8), uint8_t(topic_id & 0xFF)};
                topic_ = std::dynamic_pointer_cast<Topic>(root_objects.at(topic_xrce_id));
                topic_->tie_object(get_id());
                rv = true;
            }
            break;
        }
        default:
            break;
    }
    return rv;
}

bool DataWriter::write(dds::xrce::WRITE_DATA_Payload_Data& write_data)
{
    return middleware_->write_data(get_raw_id(), write_data.data().serialized_data());
}

bool DataWriter::matched(const dds::xrce::ObjectVariant& new_object_rep) const
{
    /* Check ObjectKind. */
    if ((get_id().at(1) & 0x0F) != new_object_rep._d())
    {
        return false;
    }

    bool rv = false;
    switch (new_object_rep.data_writer().representation()._d())
    {
        case dds::xrce::REPRESENTATION_BY_REFERENCE:
        {
            const std::string& ref = new_object_rep.data_writer().representation().object_reference();
            rv = middleware_->matched_datawriter_from_ref(get_raw_id(), ref);
            break;
        }
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml = new_object_rep.data_writer().representation().xml_string_representation();
            rv = middleware_->matched_datawriter_from_xml(get_raw_id(), xml);
            break;
        }
        default:
            break;
    }
    return rv;
}

} // namespace uxr
} // namespace eprosima
