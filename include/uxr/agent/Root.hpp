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

#ifndef UXR_AGENT_ROOT_HPP_
#define UXR_AGENT_ROOT_HPP_

#include <uxr/agent/client/ProxyClient.hpp>

#include <thread>
#include <memory>
#include <map>
#include <mutex>

namespace eprosima{
namespace uxr{

class Root
{
public:
    Root();
    ~Root();

    Root(Root&&) = delete;
    Root(const Root&) = delete;
    Root operator=(Root&&) = delete;
    Root operator=(const Root&) = delete;

    dds::xrce::ResultStatus create_client(
            const dds::xrce::CLIENT_Representation& client_representation,
            dds::xrce::AGENT_Representation& agent_representation,
            Middleware::Kind middleware_kind);

    dds::xrce::ResultStatus get_info(dds::xrce::ObjectInfo& agent_info);

    dds::xrce::ResultStatus delete_client(const dds::xrce::ClientKey& client_key);

    std::shared_ptr<ProxyClient> get_client(const dds::xrce::ClientKey& client_key);

    bool get_next_client(std::shared_ptr<ProxyClient>& next_client);

    bool load_config_file(const std::string& file_path);

    void set_verbose_level(uint8_t verbose_level);

    void reset();

private:
    std::mutex mtx_;
    std::map<dds::xrce::ClientKey, std::shared_ptr<ProxyClient>> clients_;
    std::map<dds::xrce::ClientKey, std::shared_ptr<ProxyClient>>::iterator current_client_;
};

} // uxr
} // eprosima

#endif // UXR_AGENT_ROOT_HPP_
