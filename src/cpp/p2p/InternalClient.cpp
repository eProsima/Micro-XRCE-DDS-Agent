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
#include <ucdr/microcdr.h>

#include <string>
#include <iostream>

namespace eprosima {
namespace uxr {

const uint32_t agent_key = 0xEAEAEAEA;

InternalClient::InternalClient(
        const std::array<uint8_t, 4>& ip,
        uint16_t port,
        uint32_t client_key)
    : ip_{ip}
    , port_{port}
    , domains_{}
    , topics_{}
    , topic_counter_{0}
    , transport_{}
    , platform_{}
    , client_key_{client_key}
    , session_{}
    , out_buffer_{0}
    , in_buffer_{0}
    , out_stream_id_{}
    , in_stream_id_{}
    , running_cond_{false}
    , thread_{}
{
    std::cout << "--> OK: created InternalClient at address: ";
    std::cout << int(ip[0]) << ".";
    std::cout << int(ip[1]) << ".";
    std::cout << int(ip[2]) << ".";
    std::cout << int(ip[3]);
    std::cout << " and port: " << port << std::endl;
}

static void on_topic(uxrSession* session, uxrObjectId object_id, uint16_t request_id, uxrStreamId stream_id, struct ucdrBuffer* ub, void* args)
{
    (void) session; (void) object_id; (void) request_id; (void) stream_id; (void) ub; (void) args;

    Agent::ErrorCode errcode;
    Agent::write(agent_key, Agent::get_object_id(object_id.id, Agent::DATAWRITER_OBJK), ub->iterator, ucdr_buffer_remaining(ub), errcode);
    std::cout << "Topic received" << std::endl;
}

bool InternalClient::run()
{
    bool rv = false;

    /* Set callbacks. */
    CedTopicManager::register_on_new_domain_cb(
                client_key_,
                std::bind(&InternalClient::on_new_domain, this, std::placeholders::_1));

    CedTopicManager::register_on_new_topic_cb(
                client_key_,
                std::bind(&InternalClient::on_new_topic, this, std::placeholders::_1, std::placeholders::_2));

    /* Compute IP. */
    std::string ip = std::to_string(ip_[0]) + ".";
    ip += std::to_string(ip_[1]) + ".";
    ip += std::to_string(ip_[2]) + ".";
    ip += std::to_string(ip_[3]);

    /* Create ProxyClient. */
    Agent::ErrorCode errcode;
    if (Agent::create_client(agent_key, 0x00, UXR_CONFIG_UDP_TRANSPORT_MTU, errcode))
    {
        /* Transport. */
        if (uxr_init_udp_transport(&transport_, &platform_, ip.c_str(), port_))
        {
            /* Session. */
            uxr_init_session(&session_, &transport_.comm, client_key_);
            if (uxr_create_session(&session_))
            {
                /* Set callback. */
                set_callback();

                /* Create streams. */
                create_streams();

                std::cout << "--> OK: running InternalClient" << std::endl;
                running_cond_ = true;
                thread_ = std::thread(&InternalClient::loop, this);
                rv = true;
            }
            else
            {
                std::cerr << "--> ERROR: failed to create session in InternalClient" << std::endl;
            }
        }
        else
        {
            std::cerr << "--> ERROR: failed to create transport in InternalClient" << std::endl;
        }
    }

    return rv;
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
            uint16_t agent_par_id = Agent::get_object_id(uint16_t(*it), Agent::PARTICIPANT_OBJK);
            uint16_t agent_pub_id = Agent::get_object_id(uint16_t(*it), Agent::PUBLISHER_OBJK);
            const char* ref = "";
            Agent::ErrorCode errcode;
            if (Agent::create_participant_by_ref(agent_key, agent_par_id, *it, ref, Agent::REUSE_MODE, errcode) &&
                Agent::create_publisher_by_xml(agent_key, agent_pub_id, agent_par_id, ref, Agent::REUSE_MODE, errcode))
            {
                /* Create remote entities. */
                uxrObjectId par_id = uxr_object_id(uint16_t(*it), UXR_PARTICIPANT_ID);
                uxrObjectId sub_id = uxr_object_id(uint16_t(*it), UXR_SUBSCRIBER_ID);

                uint16_t par_req = uxr_buffer_create_participant_ref(&session_, out_stream_id_, par_id, 0, ref, UXR_REUSE);
                uint16_t sub_req = uxr_buffer_create_subscriber_xml(&session_, out_stream_id_, sub_id, par_id, ref, UXR_REUSE);

                uint8_t status[2];
                uint16_t request[2] = {par_req, sub_req};

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
            Agent::ErrorCode errcode;
            uint16_t agent_par_id = Agent::get_object_id(uint16_t(it->first), Agent::PARTICIPANT_OBJK);
            uint16_t agent_top_id = Agent::get_object_id(topic_counter_, Agent::TOPIC_OBJK);
            uint16_t agent_pub_id = Agent::get_object_id(uint16_t(it->first), Agent::PUBLISHER_OBJK);
            uint16_t agent_dwt_id = Agent::get_object_id(topic_counter_, Agent::DATAWRITER_OBJK);
            if (Agent::create_topic_by_ref(agent_key, agent_top_id, agent_par_id, it->second.c_str(), Agent::REUSE_MODE, errcode) &&
                Agent::create_datawriter_by_ref(agent_key, agent_dwt_id, agent_pub_id, it->second.c_str(), Agent::REUSE_MODE, errcode))
            {
                uxrObjectId par_id = uxr_object_id(uint16_t(it->first), UXR_PARTICIPANT_ID);
                uxrObjectId top_id = uxr_object_id(uint16_t(topic_counter_), UXR_TOPIC_ID);

                uxrObjectId sub_id = uxr_object_id(uint16_t(it->first), UXR_SUBSCRIBER_ID);
                uxrObjectId drd_id = uxr_object_id(uint16_t(topic_counter_), UXR_DATAREADER_ID);

                const char* ref = it->second.c_str();

                uint16_t top_req = uxr_buffer_create_topic_ref(&session_, out_stream_id_, top_id, par_id, ref, UXR_REUSE);
                uint16_t drd_req = uxr_buffer_create_datareader_ref(&session_, out_stream_id_, drd_id, sub_id, ref, UXR_REUSE);

                /* Request data. */
                uxrDeliveryControl delivery_control = {0, 0, 0, 0};
                delivery_control.max_samples = UXR_MAX_SAMPLES_UNLIMITED;
                uxr_buffer_request_data(&session_, out_stream_id_, drd_id, in_stream_id_, &delivery_control);

                uint8_t status[2];
                uint16_t request[2] = {top_req, drd_req};

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
        uxr_run_session_until_timeout(&session_, 1000);
//        uxr_run_session_time(&session_, 1000);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
