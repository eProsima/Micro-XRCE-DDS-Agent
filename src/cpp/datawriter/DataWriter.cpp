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

std::unique_ptr<DataWriter> DataWriter::create(
        const dds::xrce::ObjectId& object_id,
        const std::shared_ptr<Publisher>& publisher,
        const dds::xrce::DATAWRITER_Representation& representation,
        const ObjectContainer& root_objects)
{
    bool created_entity = false;
    uint16_t raw_object_id = uint16_t((object_id[0] << 8) + object_id[1]);
    std::shared_ptr<Topic> topic;

    Middleware& middleware = publisher->get_middleware();
    switch (representation.representation()._d())
    {
        case dds::xrce::REPRESENTATION_BY_REFERENCE:
        {
            const std::string& ref = representation.representation().object_reference();
            uint16_t topic_id;
            if (middleware.create_datawriter_by_ref(raw_object_id, publisher->get_raw_id(), ref, topic_id))
            {
                dds::xrce::ObjectId topic_xrce_id = {uint8_t(topic_id >> 8), uint8_t(topic_id & 0xFF)};
                topic = std::dynamic_pointer_cast<Topic>(root_objects.at(topic_xrce_id));
                created_entity = true;
            }
            break;
        }
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml = representation.representation().xml_string_representation();
            uint16_t topic_id;
            if (middleware.create_datawriter_by_xml(raw_object_id, publisher->get_raw_id(), xml, topic_id))
            {
                dds::xrce::ObjectId topic_xrce_id = {uint8_t(topic_id >> 8), uint8_t(topic_id & 0xFF)};
                topic = std::dynamic_pointer_cast<Topic>(root_objects.at(topic_xrce_id));
                created_entity = true;
            }
            break;
        }
        default:
            break;
    }

    return (created_entity ? std::unique_ptr<DataWriter>(new DataWriter(object_id, publisher, topic)) : nullptr);
}

DataWriter::DataWriter(const dds::xrce::ObjectId& object_id,
        const std::shared_ptr<Publisher>& publisher,
        const std::shared_ptr<Topic>& topic)
    : XRCEObject(object_id)
    , publisher_(publisher)
    , topic_(topic)
{
    publisher_->tie_object(object_id);
    topic_->tie_object(object_id);
}

DataWriter::~DataWriter()
{
    publisher_->untie_object(get_id());
    topic_->untie_object(get_id());
    get_middleware().delete_datawriter(get_raw_id());
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
            rv = get_middleware().matched_datawriter_from_ref(get_raw_id(), ref);
            break;
        }
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml = new_object_rep.data_writer().representation().xml_string_representation();
            rv = get_middleware().matched_datawriter_from_xml(get_raw_id(), xml);
            break;
        }
        default:
            break;
    }
    return rv;
}

bool DataWriter::write(dds::xrce::WRITE_DATA_Payload_Data& write_data)
{
    return get_middleware().write_data(get_raw_id(), write_data.data().serialized_data());
}

bool DataWriter::write(std::vector<uint8_t>& data)
{
    return get_middleware().write_data(get_raw_id(), data);
}

Middleware& DataWriter::get_middleware() const
{
    return publisher_->get_middleware();
}


} // namespace uxr
} // namespace eprosima
