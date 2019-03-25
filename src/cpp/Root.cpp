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

#include <uxr/agent/Root.hpp>
#include <uxr/agent/libdev/MessageDebugger.h>
#include <uxr/agent/libdev/MessageOutput.h>
#include <uxr/agent/middleware/Middleware.hpp>
#include <uxr/agent/utils/Convertion.hpp>
#include <uxr/agent/logger/Logger.hpp>

// TODO (#5047): replace Fast RTPS dependency by XML parser library.
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <memory>
#include <chrono>

constexpr dds::xrce::XrceVendorId EPROSIMA_VENDOR_ID = {0x01, 0x0F};

namespace eprosima {
namespace uxr {

Root& Root::instance()
{
    static Root root;
    return root;
}

Root::Root()
    : mtx_(),
      clients_(),
      current_client_()
{
    current_client_ = clients_.begin();
#ifdef PROFILE_LOGGER
    spdlog::set_level(spdlog::level::trace);
    spdlog::set_pattern(UXR_LOG_PATTERN);
#endif
}

/* It must be here instead of the hpp because the forward declaration of Middleware in the hpp. */
Root::~Root() = default;

dds::xrce::ResultStatus Root::create_client(
        const dds::xrce::CLIENT_Representation& client_representation,
        dds::xrce::AGENT_Representation& agent_representation,
        Middleware::Kind middleware_kind)
{
    if (client_representation.client_key() == dds::xrce::CLIENTKEY_INVALID)
    {
        dds::xrce::ResultStatus invalid_result;
        invalid_result.status(dds::xrce::STATUS_ERR_INVALID_DATA);

        UXR_AGENT_LOG_WARN(
            UXR_CLIENT_KEY_PATTERN,
            logger::status_error("invalid client key"),
            convertion::clientkey_to_raw(client_representation.client_key()));

        return invalid_result;
    }

    dds::xrce::ResultStatus result_status;
    result_status.status(dds::xrce::STATUS_OK);

    if (client_representation.xrce_cookie() == dds::xrce::XRCE_COOKIE)
    {
        if (client_representation.xrce_version()[0] == dds::xrce::XRCE_VERSION_MAJOR)
        {
            std::lock_guard<std::mutex> lock(mtx_);
            dds::xrce::ClientKey client_key = client_representation.client_key();
            dds::xrce::SessionId session_id = client_representation.session_id();
            auto it = clients_.find(client_key);
            if (it == clients_.end())
            {
                std::shared_ptr<ProxyClient> new_client
                        = std::make_shared<ProxyClient>(client_representation, middleware_kind);
                if (clients_.emplace(client_key, std::move(new_client)).second)
                {
                    UXR_AGENT_LOG_INFO(
                        UXR_CREATE_SESSION_PATTERN,
                        logger::status_ok("created"),
                        convertion::clientkey_to_raw(client_key),
                        session_id);
                }
                else
                {
                    result_status.status(dds::xrce::STATUS_ERR_RESOURCES);

                    UXR_AGENT_LOG_INFO(
                        UXR_CLIENT_KEY_PATTERN,
                        logger::status_error("resources error"),
                        convertion::clientkey_to_raw(client_representation.client_key()));
                }
            }
            else
            {
                std::shared_ptr<ProxyClient> client = clients_.at(client_key);
                if (session_id != client->get_session_id())
                {
                    it->second = std::make_shared<ProxyClient>(client_representation, middleware_kind);
                }
                else
                {
                    client->session().reset();
                }
            }
        }
        else
        {
            result_status.status(dds::xrce::STATUS_ERR_INCOMPATIBLE);

            UXR_AGENT_LOG_INFO(
                UXR_CLIENT_KEY_PATTERN,
                logger::status_error("incompatible version"),
                convertion::clientkey_to_raw(client_representation.client_key()));
        }
    }
    else
    {
        result_status.status(dds::xrce::STATUS_ERR_INVALID_DATA);

        UXR_AGENT_LOG_INFO(
            UXR_CLIENT_KEY_PATTERN,
            logger::status_error("invalid cookie"),
            convertion::clientkey_to_raw(client_representation.client_key()));
    }

    if (dds::xrce::STATUS_OK == result_status.status())
    {
        auto epoch_time = std::chrono::duration_cast<std::chrono::nanoseconds>
                          (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        dds::xrce::Time_t timestamp;
        timestamp.seconds(static_cast<int32_t>(epoch_time / 1000000000));
        timestamp.nanoseconds(static_cast<uint32_t>(epoch_time % 1000000000));
        agent_representation.agent_timestamp(timestamp);
        agent_representation.xrce_cookie(dds::xrce::XRCE_COOKIE);
        agent_representation.xrce_version(dds::xrce::XRCE_VERSION);
        agent_representation.xrce_vendor_id(EPROSIMA_VENDOR_ID);
    }

    return result_status;
}

dds::xrce::ResultStatus Root::get_info(dds::xrce::ObjectInfo& agent_info)
{
    dds::xrce::ResultStatus result_status;

    /* Agent config. */
    auto epoch_time = std::chrono::duration_cast<std::chrono::nanoseconds>
                      (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    dds::xrce::Time_t timestamp;
    timestamp.seconds(static_cast<int32_t>(epoch_time / 1000000000));
    timestamp.nanoseconds(static_cast<uint32_t>(epoch_time % 1000000000));

    dds::xrce::AGENT_Representation agent_representation;
    agent_representation.agent_timestamp(timestamp);
    agent_representation.xrce_cookie(dds::xrce::XRCE_COOKIE);
    agent_representation.xrce_version(dds::xrce::XRCE_VERSION);
    agent_representation.xrce_vendor_id(EPROSIMA_VENDOR_ID);

    dds::xrce::ObjectVariant object_varian;
    object_varian.agent(agent_representation);

    agent_info.config(std::move(object_varian));

    return result_status;
}

dds::xrce::ResultStatus Root::delete_client(const dds::xrce::ClientKey& client_key)
{
    dds::xrce::ResultStatus result_status;
    if (get_client(client_key))
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (current_client_ != clients_.end() && client_key == current_client_->first)
        {
            ++current_client_;
        }
        clients_.erase(client_key);
        result_status.status(dds::xrce::STATUS_OK);
        UXR_AGENT_LOG_INFO(
            UXR_CLIENT_KEY_PATTERN,
            logger::status_ok("deleted"),
            convertion::clientkey_to_raw(client_key));
    }
    else
    {
        result_status.status(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);

        UXR_AGENT_LOG_INFO(
            UXR_CLIENT_KEY_PATTERN,
            logger::status_error("unknown client"),
            convertion::clientkey_to_raw(client_key));
    }
    return result_status;
}

std::shared_ptr<ProxyClient> Root::get_client(const dds::xrce::ClientKey& client_key)
{
    std::shared_ptr<ProxyClient> client;
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = clients_.find(client_key);
    if (it != clients_.end())
    {
        client = clients_.at(client_key);
    }
    return client;
}

bool Root::get_next_client(std::shared_ptr<ProxyClient>& next_client)
{
    bool rv = false;
    std::lock_guard<std::mutex> lock(mtx_);
    if (current_client_ != clients_.end())
    {
        next_client = current_client_->second;
        ++current_client_;
        rv = true;
    }
    else
    {
        current_client_ = clients_.begin();
    }
    return rv;
}

bool Root::load_config_file(const std::string& file_path)
{
    return fastrtps::xmlparser::XMLP_ret::XML_OK == fastrtps::xmlparser::XMLProfileManager::loadXMLFile(file_path);
}

void Root::reset()
{
    std::lock_guard<std::mutex> lock(mtx_);
    clients_.clear();
    current_client_ = clients_.begin();
}

} // namespace uxr
} // namespace eprosima
