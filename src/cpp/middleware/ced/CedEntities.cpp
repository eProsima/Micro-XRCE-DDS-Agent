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
#include <memory>

namespace eprosima {
namespace uxr {

/**********************************************************************************************************************
 * CedTopicManager
 **********************************************************************************************************************/
std::unordered_map<uint32_t, OnNewDomain> CedTopicManager::on_new_domain_map_;
std::unordered_map<uint32_t, OnNewTopic> CedTopicManager::on_new_topic_map_;
std::unordered_map<int16_t, std::unordered_map<std::string, std::weak_ptr<CedGlobalTopic>>> CedTopicManager::topics_;
std::mutex CedTopicManager::mtx_;

void CedTopicManager::register_on_new_domain_cb(
        uint32_t key,
        OnNewDomain on_new_domain_cb)
{
    std::lock_guard<std::mutex> lock(mtx_);
    for (auto& topic : topics_)
    {
        on_new_domain_cb(topic.first);
    }
    on_new_domain_map_.emplace(key, on_new_domain_cb);
}

void CedTopicManager::unregister_on_new_domain_cb(uint32_t key)
{
    std::lock_guard<std::mutex> lock(mtx_);
    on_new_domain_map_.erase(key);
}

void CedTopicManager::register_on_new_topic_cb(
        uint32_t key,
        OnNewTopic on_new_topic_cb)
{
    std::lock_guard<std::mutex> lock(mtx_);
    for (auto& topic : topics_)
    {
        for (auto& t : topic.second)
        {
            on_new_topic_cb(topic.first, t.first);
        }
    }
    on_new_topic_map_.emplace(key, on_new_topic_cb);
}

void CedTopicManager::unregister_on_new_topic_cb(uint32_t key)
{
    std::lock_guard<std::mutex> lock(mtx_);
    on_new_topic_map_.erase(key);
}

bool CedTopicManager::register_topic(
        const std::string& topic_name,
        int16_t domain_id,
        std::shared_ptr<CedGlobalTopic>& topic)
{
    std::unique_lock<std::mutex> lock(mtx_);
    auto it_domain = topics_.find(domain_id);
    if (topics_.end() == it_domain)
    {
        /* Call to callbacks. */
        for (auto& cb_domain : on_new_domain_map_)
        {
            cb_domain.second(domain_id);
        }

        auto it_topic = topics_[domain_id].find(topic_name);
        if (topics_[domain_id].end() == it_topic)
        {
            /* Call to callbacks. */
            for (auto& cb_topic : on_new_topic_map_)
            {
                cb_topic.second(domain_id, topic_name);
            }
            topic = std::make_shared<CedGlobalTopic>(topic_name, domain_id);
            topics_[domain_id].emplace(topic_name, topic);
        }
    }

    topic = topics_[domain_id][topic_name].lock();

    return true;
}

bool CedTopicManager::unregister_topic(
        const std::string& topic_name,
        int16_t domain_id)
{
    bool rv = false;
    std::lock_guard<std::mutex> lock(mtx_);
    auto it_domain = topics_.find(domain_id);
    if (topics_.end() != it_domain)
    {
        auto it_topic = topics_[domain_id].find(topic_name);
        if (topics_[domain_id].end() != it_topic)
        {
            if (it_topic->second.expired())
            {
                it_domain->second.erase(it_topic);
                if (it_domain->second.empty())
                {
                    topics_.erase(it_domain);
                }
                rv = true;
            }
        }
    }
    return rv;
}

/**********************************************************************************************************************
 * CedTopicCloud
 **********************************************************************************************************************/
CedGlobalTopic::CedGlobalTopic(
        const std::string& topic_name,
        int16_t domain_id)
    : name_(topic_name)
    , domain_id_(domain_id)
    , last_write_(UINT16_MAX)
{
}

CedGlobalTopic::~CedGlobalTopic()
{
    CedTopicManager::unregister_topic(name_, domain_id_);
}

const std::string& CedGlobalTopic::name() const
{
    return name_;
}

bool CedGlobalTopic::write(
        const std::vector<uint8_t>& data,
        uint8_t& errcode)
{
    std::unique_lock<std::mutex> lock(mtx_);
    size_t index = uint16_t(last_write_ + 1) % history_.size();
    history_.at(index) = data;
    ++last_write_;
    lock.unlock();
    cv_.notify_all();
    errcode = 0;
    return true;
}

bool CedGlobalTopic::read(
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout,
        SeqNum& last_read,
        uint8_t& errcode)
{
    bool rv = false;
    std::unique_lock<std::mutex> lock(mtx_);
    if (last_read != last_write_)
    {
        if ((last_read <= (last_write_ - int(history_.size()))) || (last_read > last_write_))
        {
            last_read = last_write_ - int(history_.size()) + 1;
        }
        else
        {
            ++last_read;
        }
        size_t index = uint16_t(last_read) % history_.size();
        data.assign(history_.at(index).begin(), history_.at(index).end());
        rv = true;
    }
    else
    {
        auto now = std::chrono::steady_clock::now();
        if (cv_.wait_until(lock, now + timeout, [&](){ return last_read != last_write_; }))
        {
            ++last_read;
            size_t index = uint16_t(last_read) % history_.size();
            data.assign(history_.at(index).begin(), history_.at(index).end());
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
        std::shared_ptr<CedGlobalTopic>& global_topic)
{
    bool rv = false;
    auto it = topics_.find(topic_name);
    if (topics_.end() == it)
    {
        if (CedTopicManager::register_topic(topic_name, domain_id_, global_topic))
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
        topics_.erase(it);
        rv = true;
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
    participant_->unregister_topic(global_topic_->name());
}

CedGlobalTopic* CedTopic::global_topic() const
{
    return global_topic_.get();
}

/**********************************************************************************************************************
 * CedDataWriter
 **********************************************************************************************************************/
bool CedDataWriter::write(
        const std::vector<uint8_t>& data,
        uint8_t& errcode) const
{
    return topic_->global_topic()->write(data, errcode);
}

/**********************************************************************************************************************
 * CedDataReader
 **********************************************************************************************************************/
bool CedDataReader::read(
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout,
        uint8_t &errcode)
{
    return topic_->global_topic()->read(data, timeout, last_read_, errcode);
}

} // namespace uxr
} // namespace eprosima
