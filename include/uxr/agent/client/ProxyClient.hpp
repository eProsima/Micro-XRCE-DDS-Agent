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

#ifndef _UXR_AGENT_CLIENT_PROXYCLIENT_HPP_
#define _UXR_AGENT_CLIENT_PROXYCLIENT_HPP_

#include <uxr/agent/participant/Participant.hpp>
#include <uxr/agent/client/session/Session.hpp>
#include <unordered_map>
#include <array>

namespace eprosima {
namespace uxr {

class ProxyClient
{
public:
    explicit ProxyClient(const dds::xrce::CLIENT_Representation& representation);
    ~ProxyClient() = default;

    ProxyClient(const ProxyClient&) = delete;
    ProxyClient(ProxyClient&&) = delete;
    ProxyClient& operator=(const ProxyClient&) = delete;
    ProxyClient& operator=(ProxyClient&&) = delete;

    dds::xrce::ResultStatus create(const dds::xrce::CreationMode& creation_mode,
                                   const dds::xrce::ObjectPrefix& objectid_prefix,
                                   const dds::xrce::ObjectVariant& object_representation);
    dds::xrce::ResultStatus delete_object(const dds::xrce::ObjectId& object_id);
    dds::xrce::ResultStatus update(const dds::xrce::ObjectId& object_id,
                                   const dds::xrce::ObjectVariant& representation);
    dds::xrce::ObjectInfo get_info(const dds::xrce::ObjectId& object_id);
    XRCEObject* get_object(const dds::xrce::ObjectId& object_id);
    const dds::xrce::ClientKey& get_client_key() const { return representation_.client_key(); }
    dds::xrce::SessionId get_session_id() const { return representation_.session_id(); }
    Session& session();

private:
    bool create_object(const dds::xrce::ObjectId& object_id, const dds::xrce::ObjectVariant& representation);
    bool create_participant(const dds::xrce::ObjectId& object_id,
                            const dds::xrce::OBJK_PARTICIPANT_Representation& representation);
    bool create_topic(const dds::xrce::ObjectId& object_id,
                      const dds::xrce::OBJK_TOPIC_Representation& representation);
    bool create_publisher(const dds::xrce::ObjectId& object_id,
                          const dds::xrce::OBJK_PUBLISHER_Representation& representation);
    bool create_subscriber(const dds::xrce::ObjectId& object_id,
                           const dds::xrce::OBJK_SUBSCRIBER_Representation& representation);
    bool create_datawriter(const dds::xrce::ObjectId& object_id,
                           const dds::xrce::DATAWRITER_Representation& representation);
    bool create_datareader(const dds::xrce::ObjectId& object_id,
                           const dds::xrce::DATAREADER_Representation& representation);

private:
    dds::xrce::CLIENT_Representation representation_;
    std::mutex mtx_;
    ObjectContainer objects_;
    Session session_;
};

} // namespace uxr
} // namespace eprosima

#endif //_UXR_AGENT_CLIENT_PROXYCLIENT_HPP_
