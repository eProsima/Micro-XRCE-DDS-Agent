// Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
#include <agent/datareader/DataReader.h>

#include "TokenBucket.h"
#include <fastrtps/Domain.h>
#include <xmlobjects/xmlobjects.h>

#include <fastrtps/subscriber/SampleInfo.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <atomic>

#define DEFAULT_XRCE_PARTICIPANT_PROFILE "default_xrce_participant_profile"
#define DEFAULT_XRCE_SUBSCRIBER_PROFILE "default_xrce_subscriber_profile"

namespace eprosima {
namespace micrortps {

using utils::TokenBucket;

DataReader::DataReader(const ObjectId& id, eprosima::fastrtps::Participant* rtps_participant, ReaderListener* read_list,
                       const std::string& profile_name)
    : XRCEObject(id), m_running(false), mp_reader_listener(read_list), m_rtps_subscriber_prof(profile_name),
      mp_rtps_participant(rtps_participant), mp_rtps_subscriber(nullptr), topic_type_(false)
{
}

DataReader::~DataReader() noexcept
{
    stop_read();
    if (m_read_thread.joinable())
    {
        m_read_thread.join();
    }

    if (m_max_timer_thread.joinable())
    {
        m_max_timer_thread.join();
    }

    if (nullptr != mp_rtps_subscriber)
    {
        fastrtps::Domain::removeSubscriber(mp_rtps_subscriber);
    }

    // TODO(borja): remove participant?
    if (nullptr != mp_rtps_participant)
    {
        fastrtps::Domain::removeParticipant(mp_rtps_participant);
    }
}

bool DataReader::init()
{
    if (nullptr == mp_rtps_participant &&
        nullptr == (mp_rtps_participant = fastrtps::Domain::createParticipant(DEFAULT_XRCE_PARTICIPANT_PROFILE)))
    {
        std::cout << "init participant error" << std::endl;
        return false;
    }

    SubscriberAttributes attributes;
    if (m_rtps_subscriber_prof.empty() ||
        (fastrtps::xmlparser::XMLP_ret::XML_ERROR ==
         fastrtps::xmlparser::XMLProfileManager::fillSubscriberAttributes(m_rtps_subscriber_prof, attributes)))
    {
        fastrtps::xmlparser::XMLProfileManager::getDefaultSubscriberAttributes(attributes);
    }
    // topic_type_.setName(attributes.topic.getTopicDataType().data());
    // topic_type_.m_isGetKeyDefined = (attributes.topic.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY);
    // fastrtps::Domain::registerType(mp_rtps_participant, &topic_type_);

    if (!m_rtps_subscriber_prof.empty())
    {
        // std::cout << "init DataReader RTPS subscriber" << std::endl;
        mp_rtps_subscriber = fastrtps::Domain::createSubscriber(mp_rtps_participant, m_rtps_subscriber_prof, this);
    }
    else
    {
        // std::cout << "init DataReader RTPS default subscriber" << std::endl;
        mp_rtps_subscriber =
            fastrtps::Domain::createSubscriber(mp_rtps_participant, DEFAULT_XRCE_SUBSCRIBER_PROFILE, this);
    }

    if (mp_rtps_subscriber == nullptr)
    {
        std::cout << "init subscriber error" << std::endl;
        return false;
    }
    return true;
}

bool DataReader::init(const std::string& xmlrep)
{
    if (nullptr == mp_rtps_participant &&
        nullptr == (mp_rtps_participant = fastrtps::Domain::createParticipant(DEFAULT_XRCE_PARTICIPANT_PROFILE)))
    {
        std::cout << "init participant error" << std::endl;
        return false;
    }

    SubscriberAttributes attributes;
    if (xmlobjects::parse_subscriber(xmlrep.data(), xmlrep.size(), attributes))
    {
        // topic_type_.setName(attributes.topic.getTopicDataType().data());
        // fastrtps::Domain::registerType(mp_rtps_participant, &topic_type_);
        mp_rtps_subscriber = fastrtps::Domain::createSubscriber(mp_rtps_participant, attributes, this);
    }
    else
    {
        fastrtps::xmlparser::XMLProfileManager::getDefaultSubscriberAttributes(attributes);
        // topic_type_.setName(attributes.topic.getTopicDataType().data());
        // fastrtps::Domain::registerType(mp_rtps_participant, &topic_type_);

        mp_rtps_subscriber =
            fastrtps::Domain::createSubscriber(mp_rtps_participant, DEFAULT_XRCE_SUBSCRIBER_PROFILE, this);
    }
    if (mp_rtps_subscriber == nullptr)
    {
        std::cout << "init subscriber error" << std::endl;
        return false;
    }
    return true;
}

int DataReader::read(const READ_DATA_Payload& read_data)
{
    DataDeliveryConfig delivery_config = read_data.read_specification().delivery_config();
    ReadTaskInfo read_info{};
    read_info.object_ID_  = read_data.object_id();
    read_info.request_ID_ = read_data.request_id();
    switch (delivery_config._d())
    {
        case FORMAT_DATA:
        case FORMAT_SAMPLE:
            read_info.max_elapsed_time_ = 0;
            read_info.max_rate_         = 0;
            read_info.max_samples_      = 1;
            break;
        case FORMAT_DATA_SEQ:
        case FORMAT_SAMPLE_SEQ:
        case FORMAT_PACKED_SAMPLES:
            read_info.max_elapsed_time_ = delivery_config.delivery_control().max_elapsed_time();
            read_info.max_rate_         = delivery_config.delivery_control().max_rate();
            read_info.max_samples_      = delivery_config.delivery_control().max_samples();
            break;
        default:
            std::cout << "Error: read format unexpected" << std::endl;
            break;
    }

    if (!m_read_thread.joinable())
    {
        start_read(read_info);
    }
    else
    {
        std::cout << "DataReader m_read_thread busy" << std::endl;
        stop_read();
        start_read(read_info);
    }

    return 0;
}

bool DataReader::has_message() const
{
    return msg_;
}

int DataReader::start_read(const ReadTaskInfo& read_info)
{
    std::cout << "START READ" << std::endl;
    m_running = true;
    if (read_info.max_elapsed_time_ > 0)
    {
        m_max_timer_thread = std::thread(&DataReader::run_max_timer, this, read_info.max_elapsed_time_);
    }

    m_read_thread = std::thread(&DataReader::read_task, this, read_info);
    return 0;
}

int DataReader::stop_read()
{
    std::cout << "STOP READ" << std::endl;
    m_running = false;
    m_cond_var.notify_one();
    if (m_read_thread.joinable())
    {
        m_read_thread.join();
    }

    stop_max_timer();
    if (m_max_timer_thread.joinable())
    {
        m_max_timer_thread.join();
    }
    return 0;
}

// int DataReader::cancel_read(READ_DATA_Payload &read_data)
//{
//    if (m_read_thread.joinable())
//    {
//        m_read_thread = std::thread(read_task, this);
//    }
//
//    return 0;
//}

void DataReader::read_task(const ReadTaskInfo& read_info)
{
    TokenBucket rate_manager{read_info.max_rate_};
    std::cout << "Starting read_task..." << std::endl;
    uint16_t message_count                          = 0;
    std::chrono::steady_clock::time_point last_read = std::chrono::steady_clock::now();
    while (m_running)
    {
        size_t next_data_size = nextDataSize();
        if ((next_data_size != 0u) && rate_manager.get_tokens(next_data_size))
        {
            std::cout << "Read " << message_count + 1 << std::endl;
            std::vector<unsigned char> buffer;
            if (takeNextData(&buffer))
            {
                std::cout << "Read " << next_data_size << " "
                          << "in "
                          << std::chrono::duration<double>(std::chrono::steady_clock::now() - last_read).count()
                          << std::endl;
                last_read = std::chrono::steady_clock::now();
                // TODO(borja): Handle multiple reads.
                mp_reader_listener->on_read_data(read_info.object_ID_, read_info.request_ID_, buffer);
                ++message_count;
            }
        }

        if (m_running && !m_max_time_expired && message_count < read_info.max_samples_)
        {
            // TODO(borja): add some kind of timeout?
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cond_var.wait(lock);
        }
        else if (m_max_time_expired || message_count == read_info.max_samples_)
        {
            stop_max_timer();
            m_running = false;
        }
    }

    std::cout << "exiting read_task..." << std::endl;
}

void DataReader::on_max_timeout(const asio::error_code& error)
{
    std::cout << "on_timeout" << std::endl;
    if (error)
    {
        std::cout << "error" << std::endl;
    }
    else
    {
        m_max_time_expired = true;
        m_cond_var.notify_one();
    }
}

void DataReader::onNewDataMessage(eprosima::fastrtps::Subscriber* /*sub*/)
{
    m_cond_var.notify_one();
}

ReadTimeEvent::ReadTimeEvent() : m_timer_max(m_io_service_max), m_max_time_expired(false)
{
}

int ReadTimeEvent::init_max_timer(int milliseconds)
{
    m_io_service_max.reset();
    m_timer_max.expires_from_now(std::chrono::milliseconds(milliseconds));
    m_timer_max.async_wait(std::bind(&ReadTimeEvent::on_max_timeout, this, std::placeholders::_1));
    return 0;
}

void ReadTimeEvent::stop_max_timer()
{
    m_timer_max.cancel();
    m_io_service_max.stop();
}

void ReadTimeEvent::run_max_timer(int milliseconds)
{
    std::cout << "Starting run_max_timer..." << std::endl;
    init_max_timer(milliseconds);
    m_io_service_max.run();
    std::cout << "exiting run_max_timer..." << std::endl;
}

bool DataReader::takeNextData(void* data)
{
    if (nullptr == mp_rtps_subscriber)
    {
        return false;
    }
    fastrtps::SampleInfo_t info;
    bool ret = mp_rtps_subscriber->takeNextData(data, &info);
    if (ret)
    {
        std::cout << "Sample taken " << info.sample_identity.sequence_number() << std::endl;
    }
    else
    {
        std::cout << "Error taking sample" << std::endl;
    }
    return ret;
}

size_t DataReader::nextDataSize()
{
    std::vector<unsigned char> buffer;
    fastrtps::SampleInfo_t info;
    // TODO Cuidado con las configuraciones KEEPALL
    if (mp_rtps_subscriber->readNextData(&buffer, &info))
    {
        if (info_.sampleKind == ALIVE)
        {
            if (!msg_)
            {
                msg_ = true;
            }
            return buffer.size();
        }
    }
    return 0;
}

void DataReader::onSubscriptionMatched(fastrtps::Subscriber* /*sub*/, fastrtps::MatchingInfo& info)
{
    if (info.status == MATCHED_MATCHING)
    {
        matched_++;
        std::cout << "RTPS Publisher matched" << std::endl;
    }
    else
    {
        matched_--;
        std::cout << "RTPS Publisher unmatched" << std::endl;
    }
}

} /* namespace micrortps */
} /* namespace eprosima */
