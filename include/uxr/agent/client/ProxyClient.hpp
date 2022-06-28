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

#ifndef UXR_AGENT_CLIENT_PROXYCLIENT_HPP_
#define UXR_AGENT_CLIENT_PROXYCLIENT_HPP_

#include <uxr/agent/middleware/Middleware.hpp>
#include <uxr/agent/participant/Participant.hpp>
#include <uxr/agent/client/session/Session.hpp>
#include <unordered_map>
#include <array>

namespace eprosima {
namespace uxr {

class ProxyClient : public std::enable_shared_from_this<ProxyClient>
{
public:
    enum class State : uint8_t
    {
        alive,
        dead,
        to_remove
    };

    explicit ProxyClient(
            const dds::xrce::CLIENT_Representation& representation,
            Middleware::Kind middleware_kind = Middleware::Kind(0),
            std::unordered_map<std::string, std::string>&& properties = {});

    ~ProxyClient() = default;

    ProxyClient(ProxyClient&&) = delete;
    ProxyClient(const ProxyClient&) = delete;
    ProxyClient& operator=(ProxyClient&&) = delete;
    ProxyClient& operator=(const ProxyClient&) = delete;

    dds::xrce::ResultStatus create_object(
            const dds::xrce::CreationMode& creation_mode,
            const dds::xrce::ObjectPrefix& objectid_prefix,
            const dds::xrce::ObjectVariant& object_representation);

    dds::xrce::ResultStatus delete_object(
            const dds::xrce::ObjectId& object_id);

    dds::xrce::ResultStatus update(
            const dds::xrce::ObjectId& object_id,
            const dds::xrce::ObjectVariant& representation);

    dds::xrce::ObjectInfo get_info(const dds::xrce::ObjectId& object_id);

    std::shared_ptr<XRCEObject> get_object(const dds::xrce::ObjectId& object_id);

    const dds::xrce::ClientKey& get_client_key() const { return representation_.client_key(); }

    dds::xrce::SessionId get_session_id() const { return representation_.session_id(); }

    void release();

    Session& session();

    State get_state();

    void update_state(const ProxyClient::State state = State::alive);

    Middleware& get_middleware() { return *middleware_ ; };

    bool has_hard_liveliness_check() const { return hard_liveliness_check_; }

    uint8_t & get_hard_liveliness_check_tries() { return hard_liveliness_check_tries_; }
private:
    bool create_object(
            const dds::xrce::ObjectId& object_id,
            const dds::xrce::ObjectVariant& representation,
            dds::xrce::ResultStatus& result_status);

    bool create_participant(
            const dds::xrce::ObjectId& object_id,
            const dds::xrce::OBJK_PARTICIPANT_Representation& representation,
            dds::xrce::ResultStatus& result_status);

    bool create_topic(
            const dds::xrce::ObjectId& object_id,
            const dds::xrce::OBJK_TOPIC_Representation& representation,
            dds::xrce::ResultStatus& result_status);

    bool create_publisher(
            const dds::xrce::ObjectId& object_id,
            const dds::xrce::OBJK_PUBLISHER_Representation& representation,
            dds::xrce::ResultStatus& result_status);

    bool create_subscriber(
            const dds::xrce::ObjectId& object_id,
            const dds::xrce::OBJK_SUBSCRIBER_Representation& representation,
            dds::xrce::ResultStatus& result_status);

    bool create_datawriter(
            const dds::xrce::ObjectId& object_id,
            const dds::xrce::DATAWRITER_Representation& representation,
            dds::xrce::ResultStatus& result_status);

    bool create_datareader(
            const dds::xrce::ObjectId& object_id,
            const dds::xrce::DATAREADER_Representation& representation,
            dds::xrce::ResultStatus& result_status);

    bool create_requester(
            const dds::xrce::ObjectId& object_id,
            const dds::xrce::REQUESTER_Representation& representation,
            dds::xrce::ResultStatus& result_status);

    bool create_replier(
            const dds::xrce::ObjectId& object_id,
            const dds::xrce::REPLIER_Representation& representation,
            dds::xrce::ResultStatus& result_status);

    bool delete_object_unlock(
            const dds::xrce::ObjectId& object_id);

private:
    const dds::xrce::CLIENT_Representation representation_;
    std::unique_ptr<Middleware> middleware_;
    std::mutex mtx_;
    XRCEObject::ObjectContainer objects_;
    Session session_;
    std::mutex state_mtx_;
    State state_;
    std::chrono::time_point<std::chrono::steady_clock> timestamp_;
    std::unordered_map<std::string, std::string> properties_;
    std::chrono::milliseconds client_dead_time_;
    bool hard_liveliness_check_;
    uint8_t  hard_liveliness_check_tries_;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_CLIENT_PROXYCLIENT_HPP_
