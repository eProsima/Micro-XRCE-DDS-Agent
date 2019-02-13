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
std::shared_ptr<CedTopic> CedTopicManager::register_topic(
        const std::string& topic_name,
        int16_t domain_id)
{
    return topics_[domain_id][topic_name];
}

bool CedTopicManager::unregister_topic(
        const std::string& topic_name,
        int16_t domain_id)
{
    (void) topic_name;
    (void) domain_id;
    return true;
}

/**********************************************************************************************************************
 * CedDataReader
 **********************************************************************************************************************/
bool CedDataReader::read(
        ReadCallback read_cb,
        int timeout,
        uint8_t& errcode)
{
    return topic_->read(read_cb, timeout, last_read_, errcode);
}

/**********************************************************************************************************************
 * CedDataWriter
 **********************************************************************************************************************/
bool CedDataWriter::write(
        const uint8_t* buf,
        size_t len,
        uint8_t& errcode)
{
    return topic_->write(buf, len, errcode);
}

/**********************************************************************************************************************
 * CedTopic
 **********************************************************************************************************************/
CedTopic::CedTopic(
        const std::string& topic_name,
        int16_t domain_id)
    : name_(topic_name)
    , domain_id_(domain_id)
    , last_write_(UINT16_MAX)
{

}

bool CedTopic::write(
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

bool CedTopic::read(
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

} // namespace uxr
} // namespace eprosima
