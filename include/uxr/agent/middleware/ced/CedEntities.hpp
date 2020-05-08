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

#include <uxr/agent/utils/SeqNum.hpp>

#include <string>
#include <array>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <unordered_map>
#include <memory>
#include <set>

namespace eprosima {
namespace uxr {

const uint32_t INTERNAL_CLIENT_KEY = 0xEAEAEAEA;
const uint8_t EXTERNAL_CLIENT_KEY_PREFIX = 0xEA;

/**********************************************************************************************************************
 * CedController
 **********************************************************************************************************************/
enum class TopicSource : uint8_t
{
    INTERNAL = 0,
    EXTERNAL = 1
};

enum class WriteAccess : uint8_t
{
    NONE = 0,
    INTERNAL = 1,
    EXTERNAL = 2,
    COMPLETE = 3
};

enum class ReadAccess : uint8_t
{
    NONE = 0,
    INTERNAL = 1,
    EXTERNAL = 2,
    COMPLETE = 3
};

/**********************************************************************************************************************
 * CedTopicManager
 **********************************************************************************************************************/
class CedGlobalTopic;
typedef const std::function<void (int16_t)> OnNewDomain;
typedef const std::function<void (int16_t, const std::string&)> OnNewTopic;

class CedTopicManager
{
    friend class CedGlobalTopic;
public:
    static void register_on_new_domain_cb(
            uint32_t key,
            const OnNewDomain& on_new_domain_cb);

    static void unregister_on_new_domain_cb(
            uint32_t key);

    static void register_on_new_topic_cb(
            uint32_t key,
            const OnNewTopic& on_new_topic_cb);

    static void unregister_on_new_topic_cb(
            uint32_t key);

    static bool register_topic(
            const std::string& topic_name,
            int16_t domain_id,
            std::shared_ptr<CedGlobalTopic>& topic);

private:
    CedTopicManager() = default;
    ~CedTopicManager() = default;

    static bool unregister_topic(
            const std::string& topic_name,
            int16_t domain_id);

private:
    static std::unordered_map<uint32_t, OnNewDomain> on_new_domain_map_;
    static std::unordered_map<uint32_t, OnNewTopic> on_new_topic_map_;
    static std::unordered_map<int16_t, std::unordered_map<std::string, std::weak_ptr<CedGlobalTopic>>> topics_;
    static std::mutex mtx_;
};

/**********************************************************************************************************************
 * CedTopicCloud
 **********************************************************************************************************************/
class CedGlobalTopic
{
    friend class CedDataReader;
    friend class CedDataWriter;
public:
    CedGlobalTopic(
            const std::string& topic_name,
            int16_t domain_id);

    ~CedGlobalTopic();

    const std::string& name() const;

private:
    bool write(
            const std::vector<uint8_t>& data,
            WriteAccess write_access,
            TopicSource topic_src,
            uint8_t& errcode);

    bool read(
            std::vector<uint8_t>& data,
            std::chrono::milliseconds timeout,
            SeqNum& last_read,
            ReadAccess read_access,
            uint8_t& errcode);

    bool check_write_access(
            WriteAccess write_access,
            TopicSource topic_src);

    bool check_read_access(
            ReadAccess read_access,
            size_t index);

    bool get_data(
            std::vector<uint8_t>& data,
            SeqNum& last_read,
            ReadAccess read_access);

private:
    const std::string name_;
    int16_t domain_id_;
    SeqNum last_write_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::array<std::vector<uint8_t>, 16> history_; // TODO (review history size)
    std::array<TopicSource, 16> srcs_; // TODO (review history size)
};

/**********************************************************************************************************************
 * CedParticipant
 **********************************************************************************************************************/
class CedTopic;

class CedParticipant
{
public:
    CedParticipant(int16_t domain_id)
        : domain_id_(domain_id)
    {}
    ~CedParticipant() = default;

    bool register_topic(
            const std::shared_ptr<CedTopic>& topic);

    bool unregister_topic(
            const std::string& topic_name);

    std::shared_ptr<CedTopic> find_topic(
            const std::string& topic_name) const;

    int16_t get_domain_id() const { return domain_id_; }

private:
    int16_t domain_id_;
    std::unordered_map<std::string, std::weak_ptr<CedTopic>> topic_register_;
};

/**********************************************************************************************************************
 * CedTopic
 **********************************************************************************************************************/
class CedTopic
{
public:
    CedTopic(
            const std::shared_ptr<CedParticipant>& participant,
            const std::shared_ptr<CedGlobalTopic>& global_topic)
        : participant_(participant)
        , global_topic_(global_topic)
    {}
    ~CedTopic();

    CedGlobalTopic* get_global_topic() const;

    const std::string& get_name() { return global_topic_->name(); }

private:
    std::shared_ptr<CedParticipant> participant_;
    std::shared_ptr<CedGlobalTopic> global_topic_;
};

/**********************************************************************************************************************
 * CedPublisher
 **********************************************************************************************************************/
class CedPublisher
{
    friend class CedDataWriter;
public:
    CedPublisher(const std::shared_ptr<CedParticipant>& participant)
        : participant_(participant)
    {}
    ~CedPublisher() = default;

    const std::shared_ptr<CedParticipant>& get_participant() const { return participant_; }

private:
    const std::shared_ptr<CedParticipant> participant_;
};

/**********************************************************************************************************************
 * CedSubscriber
 **********************************************************************************************************************/
class CedSubscriber
{
    friend class CedDataReader;
public:
    CedSubscriber(const std::shared_ptr<CedParticipant>& participant)
        : participant_(participant)
    {}
    ~CedSubscriber() = default;

    const std::shared_ptr<CedParticipant>& get_participant() const { return  participant_; }

private:
    const std::shared_ptr<CedParticipant>& participant_;
};

/**********************************************************************************************************************
 * CedDataWriter
 **********************************************************************************************************************/
class CedDataWriter
{
public:
    CedDataWriter(
            const std::shared_ptr<CedPublisher>& publisher,
            const std::shared_ptr<CedTopic>& topic,
            const WriteAccess write_access,
            const TopicSource topic_src)
        : publisher_(publisher)
        , topic_(topic)
        , write_access_(write_access)
        , topic_src_(topic_src)
    {}
    ~CedDataWriter() = default;

    bool write(
        const std::vector<uint8_t>& data,
        uint8_t& errcode) const;

    const std::string& topic_name() const { return topic_->get_global_topic()->name(); }

private:
    const std::shared_ptr<CedPublisher> publisher_;
    const std::shared_ptr<CedTopic> topic_;
    const WriteAccess write_access_;
    const TopicSource topic_src_;
};

/**********************************************************************************************************************
 * CedDataReader
 **********************************************************************************************************************/
class CedDataReader
{
public:
    CedDataReader(
            const std::shared_ptr<CedSubscriber> subscriber,
            const std::shared_ptr<CedTopic> topic,
            const ReadAccess read_access)
        : subscriber_(subscriber)
        , topic_(topic)
        , last_read_(UINT16_MAX)
        , read_access_(read_access)
    {}
    ~CedDataReader() = default;

    bool read(
            std::vector<uint8_t>& data,
            std::chrono::milliseconds timeout,
            uint8_t& errcode);

    const std::string& topic_name() const { return topic_->get_global_topic()->name(); }

private:
    const std::shared_ptr<CedSubscriber> subscriber_;
    const std::shared_ptr<CedTopic> topic_;
    SeqNum last_read_;
    const ReadAccess read_access_;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_MIDDLEWARE_CED_CED_ENTITIES_HPP_
