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

#ifndef UXR_AGENT_MIDDLEWARE_CED_CED_ENTITIES_HPP_
#define UXR_AGENT_MIDDLEWARE_CED_CED_ENTITIES_HPP_

#include <string>
#include <array>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <unordered_map>
#include <memory>

namespace eprosima {
namespace uxr {

typedef std::function<void (const uint8_t* buf, size_t len)> ReadCallback;

class CedTopic;

/**********************************************************************************************************************
 * CedTopicManager
 **********************************************************************************************************************/
class CedTopicManager
{
public:
    CedTopicManager() = default;
    ~CedTopicManager() = default;

    std::shared_ptr<CedTopic> register_topic(
        const std::string& topic_name,
        int16_t domain_id);

    bool unregister_topic(
        const std::string& topic_name,
        int16_t domain_id);

private:
    std::unordered_map<int16_t, std::unordered_map<std::string, std::shared_ptr<CedTopic>>> topics_;
};

/**********************************************************************************************************************
 * CedDataReader
 **********************************************************************************************************************/
class CedDataReader
{
    CedDataReader(std::shared_ptr<CedTopic> topic)
        : last_read_(UINT16_MAX)
        , topic_(topic)
    {}
    ~CedDataReader() = default;

    bool read(
        ReadCallback read_cb,
        int timeout,
        uint8_t& errcode);

private:
    uint16_t last_read_;
    std::shared_ptr<CedTopic> topic_;
};

/**********************************************************************************************************************
 * CedDataWriter
 **********************************************************************************************************************/
class CedDataWriter
{
public:
    CedDataWriter(std::shared_ptr<CedTopic> topic)
        : topic_(topic)
    {}
    ~CedDataWriter() = default;

    bool write(
        const uint8_t* buf,
        size_t len,
        uint8_t& errcode);

private:
    std::shared_ptr<CedTopic> topic_;
};

/**********************************************************************************************************************
 * CedTopic
 **********************************************************************************************************************/
class CedTopic
{
    friend class CedDataReader;
    friend class CedDataWriter;
public:
    CedTopic(
        const std::string& topic_name,
        int16_t domain_id);

    ~CedTopic() = default;

private:
    bool write(
        const uint8_t* buf,
        size_t len,
        uint8_t& errcode
    );

    bool read(ReadCallback read_cb,
        int timeout,
        uint16_t last_read,
        uint8_t& errcode
    );

private:
    const std::string name_;
    int16_t domain_id_;
    uint16_t last_write_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::array<std::vector<uint8_t>, 16> history_; // TODO (review history size)
};


} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_MIDDLEWARE_CED_CED_ENTITIES_HPP_
