// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <uxr/agent/p2p/InternalClient.hpp>
#include <uxr/agent/middleware/ced/CedEntities.hpp>
#include <uxr/agent/Agent.hpp>
#include <uxr/agent/logger/Logger.hpp>
#include <ucdr/microcdr.h>

#include <string>
#include <iostream>

namespace eprosima {
namespace uxr {

InternalClient::InternalClient(
        Agent& agent,
        const std::array<uint8_t, 4>& ip,
        uint16_t port,
        uint32_t remote_client_key,
        uint32_t local_client_key)
    : agent_(agent)
    , ip_(ip)
    , port_{port}
    , domains_{}
    , topics_{}
    , topic_counter_{0}
    , transport_{}
    , remote_client_key_{remote_client_key}
    , local_client_key_{local_client_key}
    , session_{}
    , out_buffer_{0}
    , in_buffer_{0}
    , out_stream_id_{}
    , in_stream_id_{}
    , running_cond_{false}
    , thread_{}
{}

static void on_topic(
        uxrSession* session,
        uxrObjectId object_id,
        uint16_t request_id,
        uxrStreamId stream_id,
        struct ucdrBuffer* ub,
        uint16_t length,
        void* args)
{
    (void) session; (void) object_id; (void) request_id; (void) stream_id; (void) ub; (void) length;

    InternalClient* internal_client = reinterpret_cast<InternalClient*>(args);
    Agent::OpResult result;
    internal_client->get_agent().write(
        INTERNAL_CLIENT_KEY,
        object_id.id >> 4,
        ub->iterator,
        ucdr_buffer_remaining(ub),
        result);
}

bool InternalClient::run()
{
    if (running_cond_)
    {
        return false;
    }

    bool rv = false;

    /* Set callbacks. */
    CedTopicManager::register_on_new_domain_cb(
                remote_client_key_,
                std::bind(&InternalClient::on_new_domain, this, std::placeholders::_1));

    CedTopicManager::register_on_new_topic_cb(
                remote_client_key_,
                std::bind(&InternalClient::on_new_topic, this, std::placeholders::_1, std::placeholders::_2));

    std::string ip = std::to_string(ip_[0]) + ".";
    ip += std::to_string(ip_[1]) + ".";
    ip += std::to_string(ip_[2]) + ".";
    ip += std::to_string(ip_[3]);

    std::string port = std::to_string(port_);

    Agent::OpResult result;
    if (agent_.create_client(INTERNAL_CLIENT_KEY, 0x00, UXR_CONFIG_UDP_TRANSPORT_MTU, Middleware::Kind::CED, result))
    {
        /* Transport. */
        if (uxr_init_udp_transport(&transport_, UXR_IPv4, ip.c_str(), port.c_str()))
        {
            /* Session. */
            uxr_init_session(&session_, &transport_.comm, local_client_key_);
            if (uxr_create_session(&session_))
            {
                /* Set callback. */
                set_callback();

                /* Create streams. */
                create_streams();

                UXR_AGENT_LOG_INFO(
                    UXR_DECORATE_GREEN("connected to Agent"),
                    "address: {}:{}",
                    ip, port);

                running_cond_ = true;
                thread_ = std::thread(&InternalClient::loop, this);
                rv = true;
            }
            else
            {
                UXR_AGENT_LOG_INFO(
                    UXR_DECORATE_RED("failed to create session with Agent"),
                    "address: {}:{}",
                    ip, port);
            }
        }
        else
        {
            UXR_AGENT_LOG_INFO(
                UXR_DECORATE_RED("failed to init transport with Agent"),
                "address: {}:{}",
                ip, port);
        }
    }

    return rv;
}

bool InternalClient::stop()
{
    /* Stop thread. */
    running_cond_ = false;
    if (thread_.joinable())
    {
        thread_.join();
    }
    return true;
}

void InternalClient::set_callback()
{
    uxr_set_topic_callback(&session_, on_topic, this);
}

void InternalClient::create_streams()
{
    out_stream_id_ = uxr_create_output_reliable_stream(
                &session_,
                out_buffer_,
                sizeof(out_buffer_),
                internal_client_history);

    in_stream_id_ = uxr_create_input_reliable_stream(
                &session_,
                in_buffer_,
                sizeof(in_buffer_),
                internal_client_history);
}

void InternalClient::create_domain_entities()
{
    /* Get domains. */
    std::unique_lock<std::mutex> lock(mtx_);
    if (!domains_.empty())
    {
        std::set<int16_t> created_domains = domains_;
        lock.unlock();
        for (auto it = created_domains.begin(); it != created_domains.end();)
        {
            bool entities_created = false;

            /* Create local entities. */
            const uint16_t internal_participant_id = uint16_t(*it);
            const uint16_t internal_publisher_id = uint16_t(*it);
            const char* ref = "";
            Agent::OpResult result;
            if (agent_.create_participant_by_ref(
                        INTERNAL_CLIENT_KEY,
                        internal_participant_id,
                        *it,
                        ref,
                        Agent::REUSE_MODE,
                        result)
                    &&
                agent_.create_publisher_by_xml(
                        INTERNAL_CLIENT_KEY,
                        internal_publisher_id,
                        internal_participant_id,
                        ref,
                        Agent::REUSE_MODE,
                        result))
            {
                /* Create remote entities. */
                uxrObjectId external_participant_id = uxr_object_id(uint16_t(*it), UXR_PARTICIPANT_ID);
                uxrObjectId external_subscriber_id = uxr_object_id(uint16_t(*it), UXR_SUBSCRIBER_ID);

                uint16_t participant_request = uxr_buffer_create_participant_ref(
                            &session_,
                            out_stream_id_,
                            external_participant_id,
                            0,
                            ref,
                            UXR_REUSE);
                uint16_t subscriber_request = uxr_buffer_create_subscriber_xml(
                            &session_,
                            out_stream_id_,
                            external_subscriber_id,
                            external_participant_id,
                            ref,
                            UXR_REUSE);

                uint8_t status[2];
                uint16_t request[2] = {participant_request, subscriber_request};

                if (uxr_run_session_until_all_status(&session_, 1000, request, status, sizeof(status)))
                {
                    if (UXR_STATUS_OK == status[0] && UXR_STATUS_OK == status[1])
                    {
                        entities_created = true;
                    }
                }
                else
                {
                    std::cerr << "--> ERROR: failed to create Domain Entities in InternalClient" << std::endl;
                }
            }
            if (entities_created)
            {
                ++it;
            }
            else
            {
                it = created_domains.erase(it);
            }
        }

        lock.lock();
        for (auto& d : domains_)
        {
            domains_.erase(d);
        }
        lock.unlock();
    }
}

void InternalClient::create_topic_entities()
{
    /* Get domains. */
    std::unique_lock<std::mutex> lock(mtx_);
    if (!topics_.empty())
    {
        std::set<std::pair<int16_t, std::string>> created_topics = topics_;
        lock.unlock();
        for (auto it = created_topics.begin(); it != created_topics.end(); )
        {
            bool entities_created = false;

            /* Create local entities. */
            Agent::OpResult result;
            const uint16_t internal_paraticipant_id = uint16_t(it->first);
            const uint16_t internal_topic_id = topic_counter_;
            const uint16_t internal_publisher_id = uint16_t(it->first);
            const uint16_t internal_datawriter_id = topic_counter_;
            if (agent_.create_topic_by_ref(
                        INTERNAL_CLIENT_KEY,
                        internal_topic_id,
                        internal_paraticipant_id,
                        it->second.c_str(),
                        Agent::REUSE_MODE,
                        result)
                    &&
                agent_.create_datawriter_by_ref(
                        INTERNAL_CLIENT_KEY,
                        internal_datawriter_id,
                        internal_publisher_id,
                        it->second.c_str(),
                        Agent::REUSE_MODE,
                        result))
            {
                uxrObjectId external_participant_id = uxr_object_id(uint16_t(it->first), UXR_PARTICIPANT_ID);
                uxrObjectId external_topic_id = uxr_object_id(uint16_t(topic_counter_), UXR_TOPIC_ID);
                uxrObjectId external_subscriber_id = uxr_object_id(uint16_t(it->first), UXR_SUBSCRIBER_ID);
                uxrObjectId external_datareader_id = uxr_object_id(uint16_t(topic_counter_), UXR_DATAREADER_ID);

                const char* ref = it->second.c_str();

                uint16_t topic_request = uxr_buffer_create_topic_ref(
                            &session_,
                            out_stream_id_,
                            external_topic_id,
                            external_participant_id,
                            ref,
                            UXR_REUSE);
                uint16_t datareader_request = uxr_buffer_create_datareader_ref(
                            &session_,
                            out_stream_id_,
                            external_datareader_id,
                            external_subscriber_id,
                            ref,
                            UXR_REUSE);

                /* Request data. */
                uxrDeliveryControl delivery_control = {0, 0, 0, 0};
                delivery_control.max_samples = UXR_MAX_SAMPLES_UNLIMITED;
                uxr_buffer_request_data(
                            &session_,
                            out_stream_id_,
                            external_datareader_id,
                            in_stream_id_,
                            &delivery_control);

                uint8_t status[2];
                uint16_t request[2] = {topic_request, datareader_request};

                if (uxr_run_session_until_all_status(&session_, 1000, request, status, sizeof(status)))
                {
                    if (UXR_STATUS_OK == status[0] && UXR_STATUS_OK == status[1])
                    {
                        entities_created = true;
                    }
                }
                else
                {
                    std::cerr << "--> ERROR: failed to create Topic Entities in InternalClient" << std::endl;
                }
            }
            if (entities_created)
            {
                ++it;
                ++topic_counter_;
            }
            else
            {
                it = created_topics.erase(it);
            }
        }

        lock.lock();
        for (auto& t : topics_)
        {
            topics_.erase(t);
        }
        lock.unlock();
    }
}

void InternalClient::loop()
{

    while (running_cond_)
    {
        /* Create domain entities. */
        create_domain_entities();

        /* Create topic entities. */
        create_topic_entities();

        /* Run session. */
        uxr_run_session_time(&session_, 1000);
    }
}

void InternalClient::on_new_domain(int16_t domain)
{
    std::lock_guard<std::mutex> lock(mtx_);
    domains_.insert(domain);
}

void InternalClient::on_new_topic(
        int16_t domain_id,
        const std::string& topic_name)
{
    std::lock_guard<std::mutex> lock(mtx_);
    topics_.emplace(std::make_pair(domain_id, topic_name));
}

} // namespace eprosima
} // namespace uxr
