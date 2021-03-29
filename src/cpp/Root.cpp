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
#include <uxr/agent/middleware/Middleware.hpp>
#include <uxr/agent/utils/Conversion.hpp>
#include <uxr/agent/logger/Logger.hpp>

#ifdef UAGENT_FAST_PROFILE
// TODO (#5047): replace Fast RTPS dependency by XML parser library.
#include <fastrtps/xmlparser/XMLProfileManager.h>
#endif

#include <memory>
#include <chrono>

constexpr dds::xrce::XrceVendorId EPROSIMA_VENDOR_ID = {0x01, 0x0F};

namespace eprosima {
namespace uxr {

Root::Root()
    : mtx_(),
      clients_(),
      current_client_()
{
    current_client_ = clients_.begin();
#ifdef UAGENT_LOGGER_PROFILE
    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern(UXR_LOG_PATTERN);
#endif
}

/* It must be here instead of the hpp because the forward declaration of Middleware in the hpp. */
Root::~Root()
{
    for (auto it = clients_.begin(); it != clients_.end(); )
    {
        it->second->release();
        it = clients_.erase(it);
    }
}

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
            UXR_DECORATE_RED("invalid client key"),
            UXR_CLIENT_KEY_PATTERN,
            conversion::clientkey_to_raw(client_representation.client_key()));

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
                std::unordered_map<std::string, std::string> client_properties;

                if (client_representation.properties())
                {   
                    auto v = *client_representation.properties();
                    for (auto it_props = v.begin(); it_props != v.end(); ++it_props)
                    {
                        client_properties.insert(std::pair<std::string, std::string>(it_props->name(), it_props->value()));
                    }
                }

                std::shared_ptr<ProxyClient> new_client = std::make_shared<ProxyClient>(
                    client_representation,
                    middleware_kind,
                    std::move(client_properties));
                if (clients_.emplace(client_key, std::move(new_client)).second)
                {
                    UXR_AGENT_LOG_INFO(
                        UXR_DECORATE_GREEN("create"),
                        UXR_CREATE_SESSION_PATTERN,
                        conversion::clientkey_to_raw(client_key),
                        session_id);
                }
                else
                {
                    result_status.status(dds::xrce::STATUS_ERR_RESOURCES);

                    UXR_AGENT_LOG_INFO(
                        UXR_DECORATE_RED("resources error"),
                        UXR_CLIENT_KEY_PATTERN,
                        conversion::clientkey_to_raw(client_representation.client_key()));
                }
            }
            else
            {
                std::shared_ptr<ProxyClient> client = clients_.at(client_key);
                if (session_id != client->get_session_id())
                {
                    it->second = std::make_shared<ProxyClient>(
                        client_representation,
                        middleware_kind);
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
                UXR_DECORATE_RED("incompatible version"),
                UXR_CLIENT_KEY_PATTERN,
                conversion::clientkey_to_raw(client_representation.client_key()));
        }
    }
    else
    {
        result_status.status(dds::xrce::STATUS_ERR_INVALID_DATA);

        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_RED("invalid cookie"),
            UXR_CLIENT_KEY_PATTERN,
            conversion::clientkey_to_raw(client_representation.client_key()));
    }

    agent_representation.xrce_cookie(dds::xrce::XRCE_COOKIE);
    agent_representation.xrce_version(dds::xrce::XRCE_VERSION);
    agent_representation.xrce_vendor_id(EPROSIMA_VENDOR_ID);

    return result_status;
}

dds::xrce::ResultStatus Root::get_info(dds::xrce::ObjectInfo& agent_info)
{
    dds::xrce::ResultStatus result_status;

    /* Agent config. */
    dds::xrce::AGENT_Representation agent_representation;
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
    if (std::shared_ptr<ProxyClient> client = get_client(client_key))
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (current_client_ != clients_.end() && client_key == current_client_->first)
        {
            ++current_client_;
        }
        client->release();
        clients_.erase(client_key);
        result_status.status(dds::xrce::STATUS_OK);
        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_GREEN("delete"),
            UXR_CLIENT_KEY_PATTERN,
            conversion::clientkey_to_raw(client_key));
    }
    else
    {
        result_status.status(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);

        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_RED("unknown client"),
            UXR_CLIENT_KEY_PATTERN,
            conversion::clientkey_to_raw(client_key));
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
#ifdef UAGENT_FAST_PROFILE
    return fastrtps::xmlparser::XMLP_ret::XML_OK == fastrtps::xmlparser::XMLProfileManager::loadXMLFile(file_path);
#else
    (void) file_path;
    return false;
#endif
}

void Root::set_verbose_level(uint8_t verbose_level)
{
#ifdef UAGENT_LOGGER_PROFILE
    switch (verbose_level)
    {
        case 0:
            spdlog::set_level(spdlog::level::off);
            UXR_AGENT_LOG_WARN(
                UXR_DECORATE_YELLOW("logger off"),
                "verbose_level: {}", 0);
            break;
        case 1:
            spdlog::set_level(spdlog::level::critical);
            UXR_AGENT_LOG_INFO(
                UXR_DECORATE_GREEN("logger setup"),
                "verbose_level: {}", 1);
            break;
        case 2:
            spdlog::set_level(spdlog::level::err);
            UXR_AGENT_LOG_INFO(
                UXR_DECORATE_GREEN("logger setup"),
                "verbose_level: {}", 2);
            break;
        case 3:
            spdlog::set_level(spdlog::level::warn);
            UXR_AGENT_LOG_INFO(
                UXR_DECORATE_GREEN("logger setup"),
                "verbose_level: {}", 3);
            break;
        case 4:
            spdlog::set_level(spdlog::level::info);
            UXR_AGENT_LOG_INFO(
                UXR_DECORATE_GREEN("logger setup"),
                "verbose_level: {}", 4);
            break;
        case 5:
            spdlog::set_level(spdlog::level::debug);
            UXR_AGENT_LOG_INFO(
                UXR_DECORATE_GREEN("logger setup"),
                "verbose_level: {}", 5);
            break;
        case 6:
            spdlog::set_level(spdlog::level::trace);
            UXR_AGENT_LOG_INFO(
                UXR_DECORATE_GREEN("logger setup"),
                "verbose_level: {}", 6);
            break;
        default:
            UXR_AGENT_LOG_WARN(
                UXR_DECORATE_YELLOW("out-of-range level"),
                "verbose_level: {}", verbose_level);
            break;
    }
#else
    (void) verbose_level;
#endif
}

void Root::reset()
{
    std::lock_guard<std::mutex> lock(mtx_);
    for (auto it = clients_.begin(); it != clients_.end(); )
    {
        it->second->release();
        it = clients_.erase(it);
    }
    clients_.clear();
    current_client_ = clients_.begin();
}

} // namespace uxr
} // namespace eprosima
