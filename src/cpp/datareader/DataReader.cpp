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

/*
 * Subscriber.cpp
 *
 */
#include <agent/datareader/DataReader.h>

#include <fastrtps/Domain.h>
#include <fastrtps/subscriber/SampleInfo.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <xmlobjects/xmlobjects.h>

#include <atomic>

#define DEFAULT_XRCE_PARTICIPANT_PROFILE "default_xrce_participant_profile"
#define DEFAULT_XRCE_SUBSCRIBER_PROFILE "default_xrce_subscriber_profile"

namespace eprosima {
namespace micrortps {

DataReader::DataReader(eprosima::fastrtps::Participant* rtps_participant, ReaderListener* read_list)
    : m_running(false), mp_reader_listener(read_list), m_rtps_subscriber_prof(""),
      mp_rtps_participant(rtps_participant), mp_rtps_subscriber(nullptr)
{
    init();
}

DataReader::DataReader(const char* xmlrep, size_t size, eprosima::fastrtps::Participant* rtps_participant,
                       ReaderListener* read_list)
    : m_running(false), mp_reader_listener(read_list), m_rtps_subscriber_prof(""),
      mp_rtps_participant(rtps_participant), mp_rtps_subscriber(nullptr)
{
    init(xmlrep, size);
}

DataReader::~DataReader()
{
    if (m_read_thread.joinable())
    {
        m_read_thread.detach();
    }

    if (m_timer_thread.joinable())
    {
        m_timer_thread.detach();
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

    fastrtps::Domain::registerType(mp_rtps_participant, &m_shape_type);

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

bool DataReader::init(const char* xmlrep, size_t size)
{
    if (nullptr == mp_rtps_participant &&
        nullptr == (mp_rtps_participant = fastrtps::Domain::createParticipant(DEFAULT_XRCE_PARTICIPANT_PROFILE)))
    {
        std::cout << "init participant error" << std::endl;
        return false;
    }

    fastrtps::Domain::registerType(mp_rtps_participant, &m_shape_type);

    SubscriberAttributes attributes;
    if (xmlobjects::parse_subscriber(xmlrep, size, attributes))
    {
        mp_rtps_subscriber = fastrtps::Domain::createSubscriber(mp_rtps_participant, attributes, this);
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

int DataReader::read(const READ_DATA_Payload& read_data)
{
    switch (read_data.read_specification().delivery_config()._d())
    {
        case FORMAT_DATA:
        case FORMAT_DATA_SEQ:
        case FORMAT_SAMPLE:
        case FORMAT_SAMPLE_SEQ:
        case FORMAT_PACKED_SAMPLES:
        default:
            break;
    }

    if (!m_read_thread.joinable())
    {
        start_read(read_data);
    }
    else
    {
        // std::cout << "DataReader m_read_thread busy" << std::endl;
        stop_read();
        start_read(read_data);
    }

    return 0;
}

int DataReader::start_read(const READ_DATA_Payload& read_data)
{
    // std::cout << "START READ" << std::endl;
    m_running      = true;
    m_timer_thread = std::thread(&DataReader::run_timer, this);
    m_read_thread  = std::thread(&DataReader::read_task, this, read_data);
    return 0;
}

int DataReader::stop_read()
{
    // std::cout << "STOP READ" << std::endl;
    m_running = false;
    m_cond_var.notify_one();
    m_timer_thread.join();
    m_read_thread.join();
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

void DataReader::read_task(READ_DATA_Payload read_data)
{
    // std::cout << "Starting read_task..." << std::endl;
    std::unique_lock<std::mutex> lock(m_mutex);
    uint16_t message_count = 0;
    while (m_running)
    {
        m_cond_var.wait(lock, [&] { return !m_running || (m_time_expired && m_new_message); });

        std::cout << "Read " << message_count + 1 << std::endl;

        std::vector<unsigned char> buffer;
        if (takeNextData(&buffer))
        {
            mp_reader_listener->on_read_data(read_data.object_id(), read_data.request_id(), buffer);
            ++message_count;
        }

        m_time_expired = m_new_message = false;
    }
    m_running = false;
    // std::cout << "exiting read_task..." << std::endl;
}

void DataReader::on_timeout(const asio::error_code& error)
{
    // std::cout << "on_timeout" << std::endl;
    if (error)
    {
        std::cout << "error" << std::endl;
    }
    else
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_time_expired = true;
        m_cond_var.notify_one();

        // Relaunch timer
        if (m_running)
        {
            init_timer(2000);
        }
    }
}

void DataReader::onNewDataMessage(eprosima::fastrtps::Subscriber* sub)
{
    // Take data
    std::vector<unsigned char> buffer;
    if (sub->readNextData(&buffer, &m_info))
    {
        if (m_info.sampleKind == ALIVE)
        {
            // Print your structure data here.
            ++n_msg;
            // std::cout << "Sample received " << m_info.sample_identity.sequence_number() << std::endl;

            std::lock_guard<std::mutex> lock(m_mutex);
            m_new_message = true;
            m_cond_var.notify_one();
        }
    }
}

TimerEvent::TimerEvent() : m_timer(m_io_service)
{
}

int TimerEvent::init_timer(int milliseconds)
{
    m_io_service.reset();
    m_timer.expires_from_now(std::chrono::milliseconds(milliseconds));
    m_timer.async_wait(std::bind(&TimerEvent::on_timeout, this, std::placeholders::_1));
    return 0;
}

void TimerEvent::run_timer()
{
    // std::cout << "Starting run_timer..." << std::endl;
    init_timer(2000);
    m_io_service.run();
    // std::cout << "exiting run_timer..." << std::endl;
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
        // std::cout << "Sample taken " << info.sample_identity.sequence_number() << std::endl;
    }
    else
    {
        std::cout << "Error taken sample" << std::endl;
    }
    return ret;
}

void DataReader::onSubscriptionMatched(fastrtps::MatchingInfo& info)
{
    if (info.status == MATCHED_MATCHING)
    {
        n_matched++;
        // std::cout << "RTPS Publisher matched" << std::endl;
    }
    else
    {
        n_matched--;
        // std::cout << "RTPS Publisher unmatched" << std::endl;
    }
}

} /* namespace micrortps */
} /* namespace eprosima */
