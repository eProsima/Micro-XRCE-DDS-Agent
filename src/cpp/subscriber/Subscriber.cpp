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

#include <uxr/agent/subscriber/Subscriber.hpp>
#include <uxr/agent/client/ProxyClient.hpp>

namespace eprosima {
namespace uxr {

std::unique_ptr<Subscriber> Subscriber::create(
        const dds::xrce::ObjectId& object_id,
        uint16_t participant_id,
        const std::shared_ptr<ProxyClient>& proxy_client,
        const dds::xrce::OBJK_SUBSCRIBER_Representation& representation)
{
    bool created_entity = false;
    uint16_t raw_object_id = conversion::objectid_to_raw(object_id);

    Middleware& middleware = proxy_client->get_middleware();
    switch (representation.representation()._d())
    {
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml = representation.representation().string_representation();
            created_entity = middleware.create_subscriber_by_xml(raw_object_id, participant_id, xml);
            break;
        }
        case dds::xrce::REPRESENTATION_IN_BINARY:
        {
            auto rep = representation.representation();
            dds::xrce::OBJK_Subscriber_Binary subscriber_xrce;

            fastcdr::FastBuffer fastbuffer{reinterpret_cast<char*>(const_cast<uint8_t*>(rep.binary_representation().data())), rep.binary_representation().size()};
            eprosima::fastcdr::Cdr::Endianness endianness = static_cast<eprosima::fastcdr::Cdr::Endianness>(representation.endianness());
            eprosima::fastcdr::Cdr cdr(fastbuffer, endianness, eprosima::fastcdr::CdrVersion::XCDRv1);
            subscriber_xrce.deserialize(cdr);

            created_entity = proxy_client->get_middleware().create_subscriber_by_bin(raw_object_id, participant_id, subscriber_xrce);
            break;
        }
    }

    return (created_entity ? std::unique_ptr<Subscriber>(new Subscriber(object_id, proxy_client)) : nullptr);
}

Subscriber::Subscriber(
        const dds::xrce::ObjectId& object_id,
        const std::shared_ptr<ProxyClient>& proxy_client)
    : XRCEObject{object_id}
    , proxy_client_{proxy_client}
{}

Subscriber::~Subscriber()
{
    proxy_client_->get_middleware().delete_subscriber(get_raw_id());
}

} // namespace uxr
} // namespace eprosima
