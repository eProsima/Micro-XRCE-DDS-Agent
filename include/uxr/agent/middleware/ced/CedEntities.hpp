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

namespace eprosima {
namespace uxr {

class CedTopicImpl;

/**********************************************************************************************************************
 * CedTopicManager
 **********************************************************************************************************************/
class CedTopicManager
{
    friend class CedParticipant;
private:
    CedTopicManager() = default;
    ~CedTopicManager() = default;

    static bool register_topic(
        const std::string& topic_name,
        int16_t domain_id,
        std::shared_ptr<CedTopicImpl>& topic);

    static bool unregister_topic(
        const std::string& topic_name,
        int16_t domain_id);

private:
    static std::unordered_map<int16_t, std::unordered_map<std::string, std::shared_ptr<CedTopicImpl>>> topics_;
    static std::mutex mtx_;
};

/**********************************************************************************************************************
 * CedTopicCloud
 **********************************************************************************************************************/
class CedTopicImpl
{
    friend class CedDataReader;
    friend class CedDataWriter;
public:
    CedTopicImpl(
        const std::string& topic_name,
        int16_t domain_id);

    ~CedTopicImpl() = default;

    const std::string& name();

private:
    bool write(
        const std::vector<uint8_t>& data,
        uint8_t& errcode
    );

    bool read(std::vector<uint8_t>& data,
        uint32_t timeout,
        SeqNum& last_read,
        uint8_t& errcode
    );

private:
    const std::string name_;
    int16_t domain_id_;
    SeqNum last_write_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::array<std::vector<uint8_t>, 16> history_; // TODO (review history size)
};

/**********************************************************************************************************************
 * CedParticipant
 **********************************************************************************************************************/
class CedParticipant
{
public:
    CedParticipant(int16_t domain_id)
        : domain_id_(domain_id)
    {}
    ~CedParticipant() = default;

    bool register_topic(
            const std::string& topic_name,
            uint16_t topic_id,
            std::shared_ptr<CedTopicImpl>& topic_impl);

    bool unregister_topic(const std::string& topic_name);

    bool find_topic(
            const std::string& topic_name,
            uint16_t& topic_id) const;

    int16_t domain_id() const { return domain_id_; }

private:
    int16_t domain_id_;
    std::unordered_map<std::string, uint16_t> topics_;
};

/**********************************************************************************************************************
 * CedTopic
 **********************************************************************************************************************/
class CedTopic
{
public:
    CedTopic(
            const std::shared_ptr<CedParticipant>& participant,
            const std::shared_ptr<CedTopicImpl>& topic_impl)
        : participant_(participant)
        , topic_impl_(topic_impl)
    {}
    ~CedTopic();

    const std::shared_ptr<CedTopicImpl>& topic_impl() const;

    const std::string& name() { return topic_impl_->name(); }

private:
    std::shared_ptr<CedParticipant> participant_;
    std::shared_ptr<CedTopicImpl> topic_impl_;
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

    const std::shared_ptr<CedParticipant>& participant() const { return participant_; }

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

    const std::shared_ptr<CedParticipant>& participant() const { return  participant_; }

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
            const std::shared_ptr<CedTopic>& topic)
        : publisher_(publisher)
        , topic_(topic)
    {}
    ~CedDataWriter() = default;

    bool write(
        const std::vector<uint8_t>& data,
        uint8_t& errcode) const;

    const std::string& topic_name() const { return topic_->topic_impl()->name(); }

private:
    const std::shared_ptr<CedPublisher> publisher_;
    const std::shared_ptr<CedTopic> topic_;
};

/**********************************************************************************************************************
 * CedDataReader
 **********************************************************************************************************************/
class CedDataReader
{
public:
    CedDataReader(
            const std::shared_ptr<CedSubscriber> subscriber,
            const std::shared_ptr<CedTopic> topic)
        : subscriber_(subscriber)
        , topic_(topic)
        , last_read_(UINT16_MAX)
    {}
    ~CedDataReader() = default;

    bool read(
            std::vector<uint8_t>& data,
            uint32_t timeout,
            uint8_t& errcode);

    const std::string& topic_name() const { return topic_->topic_impl()->name(); }

private:
    const std::shared_ptr<CedSubscriber> subscriber_;
    const std::shared_ptr<CedTopic> topic_;
    SeqNum last_read_;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_MIDDLEWARE_CED_CED_ENTITIES_HPP_
