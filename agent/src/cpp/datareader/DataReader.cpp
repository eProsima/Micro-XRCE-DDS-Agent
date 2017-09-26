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
#include <fastrtps/subscriber/Subscriber.h>
#include <atomic>

#define DEFAULT_XRCE_PARTICIPANT_PROFILE "default_xrce_participant_profile"
#define DEFAULT_XRCE_SUBSCRIBER_PROFILE "default_xrce_subscriber_profile"



namespace eprosima {
namespace micrortps {

DataReader::DataReader(eprosima::fastrtps::Participant* rtps_participant, ReaderListener* read_list):
        mp_reader_listener(read_list),
        mp_rtps_participant(rtps_participant),
        mp_rtps_subscriber(nullptr),
        m_rtps_subscriber_prof("")
{
    init();
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

    // TODO: remove participant?
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
        printf("init participant error\n");
        return false;
    }

    fastrtps::Domain::registerType(mp_rtps_participant,(fastrtps::TopicDataType*) &m_shape_type);

    if (!m_rtps_subscriber_prof.empty())
    {
        //printf("init DataReader RTPS subscriber\n");
        mp_rtps_subscriber = fastrtps::Domain::createSubscriber(mp_rtps_participant, m_rtps_subscriber_prof, this);
    }
    else
    {
        //printf("init DataReader RTPS default subscriber\n");
        mp_rtps_subscriber = fastrtps::Domain::createSubscriber(mp_rtps_participant, DEFAULT_XRCE_SUBSCRIBER_PROFILE, this);
    }

    if(mp_rtps_subscriber == nullptr)
    {
        printf("init subscriber error\n");
        return false;
    }
    return true;
}

int DataReader::read(const READ_DATA_PAYLOAD& read_data)
{
    switch(read_data.read_mode())
    {
        case READM_DATA: break;
        case READM_SAMPLE: break;
        case READM_DATA_SEQ: break;
        case READM_SAMPLE_SEQ: break;
        case READM_PACKED_SAMPLE_SEQ: break;
        default: break;
    }


    if (!m_read_thread.joinable())
    {
        start_read(read_data);
    }
    else
    {
        //printf("DataReader m_read_thread busy\n");
        stop_read();
        start_read(read_data);
    }

    return 0;
}

int DataReader::start_read(const READ_DATA_PAYLOAD& read_data)
{
    //printf("START READ\n");
    m_running = true;
    m_timer_thread = std::thread(&DataReader::run_timer, this);
    m_read_thread = std::thread(&DataReader::read_task, this, read_data);
    return 0;
}

int DataReader::stop_read()
{
    //printf("STOP READ\n");
    m_running = false;
    m_cond_var.notify_one();
    m_timer_thread.join();
    m_read_thread.join();
    return 0;
}

//int DataReader::cancel_read(READ_DATA_PAYLOAD &read_data)
//{
//    if (m_read_thread.joinable())
//    {
//        m_read_thread = std::thread(read_task, this);
//    }
//
//    return 0;
//}

void DataReader::read_task(READ_DATA_PAYLOAD read_data)
{
    //printf("Starting read_task...\n");
    std::unique_lock<std::mutex> lock(m_mutex);
    uint16_t message_count = 0;
    while(m_running && (0 == read_data.max_messages() || message_count < read_data.max_messages()))
    {
        m_cond_var.wait(lock, [&]{return !m_running || (m_time_expired && m_new_message);});

        printf("Read %u of %u\n", message_count + 1, read_data.max_messages());

        std::vector<unsigned char> buffer;
        if (takeNextData(&buffer))
        {
            mp_reader_listener->on_read_data(read_data.object_id(), read_data.request_id(), buffer);
            ++message_count;
        }

        m_time_expired = m_new_message = false;
    }
    m_running = false;
    //printf("exiting read_task...\n");
}

void DataReader::on_timeout(const asio::error_code& error)
{
    //printf("on_timeout\n");
    if (error)
    {
        printf("error\n");
    }
    else
    {
        std::lock_guard < std::mutex > lock(m_mutex);
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
            //std::cout << "Sample received " << m_info.sample_identity.sequence_number() << std::endl;

            std::lock_guard<std::mutex> lock(m_mutex);
            m_new_message = true;
            m_cond_var.notify_one();
        }
    }
}

TimerEvent::TimerEvent(): m_timer(m_io_service)
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
    //printf("Starting run_timer...\n");
    init_timer(2000);
    m_io_service.run();
    //printf("exiting run_timer...\n");
}

bool DataReader::takeNextData(void* data)
{
    if (nullptr == mp_rtps_subscriber)
    {
        return false;
    }
    fastrtps::SampleInfo_t info;
    bool ret = mp_rtps_subscriber->takeNextData(data, &info);
    if(ret)
    {
        //std::cout << "Sample taken " << info.sample_identity.sequence_number() << std::endl;
    }
    else
    {
        std::cout << "Error taken sample" << std::endl;
    }
    return ret;
}

void DataReader::onSubscriptionMatched(fastrtps::Subscriber* sub, fastrtps::MatchingInfo& info)
{
    if (info.status == MATCHED_MATCHING)
    {
        n_matched++;
        //std::cout << "RTPS Publisher matched" << std::endl;
    }
    else
    {
        n_matched--;
        //std::cout << "RTPS Publisher unmatched" << std::endl;
    }
}



} /* namespace micrortps */
} /* namespace eprosima */


