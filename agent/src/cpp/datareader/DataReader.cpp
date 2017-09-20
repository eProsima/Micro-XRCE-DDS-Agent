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

#include <fastrtps/Domain.h>
#include <fastrtps/subscriber/SampleInfo.h>

#include <agent/datareader/DataReader.h>

#define DEFAULT_XRCE_PARTICIPANT_PROFILE "default_xrce_participant_profile"
#define DEFAULT_XRCE_SUBSCRIBER_PROFILE "default_xrce_subscriber_profile"



namespace eprosima {
namespace micrortps {

DataReader::DataReader(ReaderListener* read_list):
        mp_reader_listener(read_list),
        mp_rtps_participant(nullptr),
        mp_rtps_subscriber(nullptr),
        m_rtps_subscriber_prof("")
{
    init();
}

DataReader::~DataReader()
{
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
        return false;
    }

    fastrtps::Domain::registerType(mp_rtps_participant,(fastrtps::TopicDataType*) &m_shape_type);

    if (!m_rtps_subscriber_prof.empty())
    {
        mp_rtps_subscriber = fastrtps::Domain::createSubscriber(mp_rtps_participant, m_rtps_subscriber_prof, &m_rtps_listener);
    }
    else
    {
        mp_rtps_subscriber = fastrtps::Domain::createSubscriber(mp_rtps_participant, DEFAULT_XRCE_SUBSCRIBER_PROFILE, &m_rtps_listener);
    }

    if(mp_rtps_subscriber == nullptr)
    {
        return false;
    }
    return true;
}

int DataReader::read(READ_DATA_PAYLOAD &read_data)
{
    switch(read_data.read_mode())
    {
        case READM_DATA:
        case READM_SAMPLE:
            if (!m_read_thread.joinable())
            {
                m_read_thread = std::thread(&DataReader::read_task, this);
            }
        break;
        case READM_DATA_SEQ: break;
        case READM_SAMPLE_SEQ: break;
        case READM_PACKED_SAMPLE_SEQ: break;
        default: break;
    }

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

void DataReader::read_task()
{
    std::unique_lock<std::mutex> lock(mutx);
    int pos = 0;
    while(true) // running
    {
        cond_var.wait(lock, [&]{return time_expired && m_rtps_listener.new_message;});

        ShapeType st;
        takeNextData(&st);
        mp_reader_listener->on_read_data(read_data.object_id(), read_data.request_id(), (octet*)&st, sizeof(ShapeType));

        time_expired = m_rtps_listener.new_message = false;
    }
    printf("exiting read_task...\n");
    return 0;
}

void DataReader::on_timeout(const asio::error_code& error)
{
  if (error)
  {
      printf("error");
  }
  else
  {
      printf("Time expired\n");
      std::lock_guard<std::mutex> lock(mutx);
      time_expired = true;
      cond_var.notify_one();
      timr->expires_from_now(std::chrono::milliseconds(3000));
      timr->async_wait(std::bind(&timer::on_timeout, this, std::placeholders::_1));
  }

}

timer::timer()
{
    timr = new asio::steady_timer(io_serv, std::chrono::milliseconds(1000));
    timr->async_wait(std::bind(&timer::on_timeout, this, std::placeholders::_1));
}

void timer::run()
{
    io_serv.run();
}

bool DataReader::readNextData(void* data)
{
    if (nullptr == mp_rtps_subscriber)
    {
        return false;
    }
    fastrtps::SampleInfo_t info;
    return mp_rtps_subscriber->readNextData(data, &info);
}

bool DataReader::takeNextData(void* data)
{
    if (nullptr == mp_rtps_subscriber)
    {
        return false;
    }
    fastrtps::SampleInfo_t info;
    return mp_rtps_subscriber->takeNextData(data, &info);
}

void DataReader::RTPSSubListener::onSubscriptionMatched(fastrtps::Subscriber* sub, fastrtps::MatchingInfo& info)
{
    if (info.status == MATCHED_MATCHING)
    {
        n_matched++;
        std::cout << "Subscriber matched" << std::endl;
    }
    else
    {
        n_matched--;
        std::cout << "Subscriber unmatched" << std::endl;
    }
}

void DataReader::RTPSSubListener::onNewDataMessage(fastrtps::Subscriber* sub)
{
    // Take data
    ShapeType st;

    if (sub->takeNextData(&st, &m_info))
    {
        if (m_info.sampleKind == ALIVE)
        {
            // Print your structure data here.
            ++n_msg;
            std::cout << "Sample received, count=" << n_msg << std::endl;

            std::lock_guard<std::mutex> lock(mutx);
            new_message = true;
            cond_var.notify_one();
        }
    }
}

} /* namespace micrortps */
} /* namespace eprosima */


