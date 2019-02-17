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

#include <uxr/agent/middleware/ced/CedEntities.hpp>
#include <chrono>

namespace eprosima {
namespace uxr {

/**********************************************************************************************************************
 * CedTopicManager
 **********************************************************************************************************************/
std::unordered_map<int16_t, std::unordered_map<std::string, std::shared_ptr<CedTopicImpl>>> CedTopicManager::topics_;

bool CedTopicManager::register_topic(
        const std::string& topic_name,
        int16_t domain_id,
        std::shared_ptr<CedTopicImpl>& topic)
{
    auto it = topics_[domain_id].find(topic_name);
    if (topics_[domain_id].end() == it)
    {
        topic = std::shared_ptr<CedTopicImpl>(new CedTopicImpl(topic_name, domain_id));
        topics_[domain_id][topic_name] = topic;
    }
    else
    {
        topic = topics_[domain_id][topic_name];
    }
    return true;
}

bool CedTopicManager::unregister_topic(
        const std::string& topic_name,
        int16_t domain_id)
{
    bool rv = false;
    auto it_domain = topics_.find(domain_id);
    if (topics_.end() != it_domain)
    {
        auto it_topic = topics_[domain_id].find(topic_name);
        if (topics_[domain_id].end() != it_topic)
        {
            rv = true; // TODO
        }
    }
    return rv;
}

/**********************************************************************************************************************
 * CedTopicCloud
 **********************************************************************************************************************/
CedTopicImpl::CedTopicImpl(
        const std::string& topic_name,
        int16_t domain_id)
    : name_(topic_name)
    , domain_id_(domain_id)
    , last_write_(UINT16_MAX)
{}

const std::string& CedTopicImpl::name()
{
    return name_;
}

bool CedTopicImpl::write(
        const uint8_t* buf,
        size_t len,
        uint8_t& errcode)
{
    std::unique_lock<std::mutex> lock(mtx_);
    size_t index = uint16_t(last_write_ + 1) % history_.size();
    history_[index] = std::vector<uint8_t>(buf, buf + len);
    ++last_write_;
    lock.unlock();
    cv_.notify_all();
    errcode = 0;
    return true;
}

bool CedTopicImpl::read(
        ReadCallback read_cb,
        int timeout,
        uint16_t last_read,
        uint8_t& errcode)
{
    bool rv = false;
    std::unique_lock<std::mutex> lock(mtx_);
    if (last_read != last_write_)
    {
        // TODO (add SeqNum)
    }
    else
    {
        auto now = std::chrono::steady_clock::now();
        if (cv_.wait_until(lock, now + std::chrono::milliseconds(timeout), [&](){ return last_read != last_write_; }))
        {
            size_t index = uint16_t(last_read + 1) % history_.size();
            const std::vector<uint8_t> message = history_[index];
            read_cb(message.data(), message.size());
            rv = true;
        }
        else
        {
            errcode = 1;
        }
    }
    return rv;
}


/**********************************************************************************************************************
 * CedParticipant
 **********************************************************************************************************************/
bool CedParticipant::register_topic(
        const std::string& topic_name,
        uint16_t topic_id,
        std::shared_ptr<CedTopicImpl>& topic_impl)
{
    bool rv = false;
    auto it = topics_.find(topic_name);
    if (topics_.end() == it)
    {
        if (CedTopicManager::register_topic(topic_name, domain_id_, topic_impl))
        {
            topics_.emplace(topic_name, topic_id);
            rv = true;
        }
    }
    return rv;
}

bool CedParticipant::unregister_topic(const std::string &topic_name)
{
    bool rv = false;
    auto it = topics_.find(topic_name);
    if (topics_.end() != it)
    {
        if (CedTopicManager::unregister_topic(topic_name, domain_id_))
        {
            topics_.erase(topic_name);
            rv = true;
        }
    }
    return rv;
}

bool CedParticipant::find_topic(
        const std::string& topic_name,
        uint16_t& topic_id) const
{
    bool rv = false;
    auto it = topics_.find(topic_name);
    if (topics_.end() != it)
    {
        topic_id = it->second;
        rv = true;
    }
    return rv;
}

/**********************************************************************************************************************
 * CedTopic
 **********************************************************************************************************************/
CedTopic::~CedTopic()
{
    participant_->unregister_topic(topic_impl_->name());
}

const std::shared_ptr<CedTopicImpl>& CedTopic::topic_impl() const
{
    return topic_impl_;
}

/**********************************************************************************************************************
 * CedDataWriter
 **********************************************************************************************************************/
bool CedDataWriter::write(
        const uint8_t* buf,
        size_t len,
        uint8_t& errcode) const
{
    return topic_->topic_impl()->write(buf, len, errcode);
}

/**********************************************************************************************************************
 * CedDataReader
 **********************************************************************************************************************/
bool CedDataReader::read(
        ReadCallback read_cb,
        int timeout,
        uint8_t &errcode)
{
    return topic_->topic_impl()->read(read_cb, timeout, last_read_, errcode);
}

} // namespace uxr
} // namespace eprosima
