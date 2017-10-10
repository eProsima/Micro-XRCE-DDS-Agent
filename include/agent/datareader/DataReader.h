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

/**
 * @file DataReader.h
 */


#ifndef DATAREADER_H_
#define DATAREADER_H_

#include <thread>
#include <mutex>
#include <condition_variable>
#include <asio/io_service.hpp>
#include <asio/steady_timer.hpp>

#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/subscriber/SampleInfo.h>
#include <fastrtps/rtps/common/MatchingInfo.h>

#include <agent/types/ShapePubSubTypes.h>
#include <DDSXRCETypes.h>
#include <Payloads.h>
#include <agent/Common.h>

namespace eprosima {

namespace fastrtps {
    class Participant;
    class Subscriber;
}

namespace micrortps {

class ReaderListener
{
public:
    ReaderListener() = default;
    virtual ~ReaderListener() = default;

    virtual void on_read_data(const ObjectId& object_id, const RequestId& req_id,
            const std::vector<unsigned char>& buffer) = 0;
};

class TimerEvent
{
public:
    TimerEvent();
    virtual ~TimerEvent() = default;
    void run_timer();
    int init_timer(int milliseconds);

    virtual void on_timeout(const asio::error_code& error) = 0;

protected:
    asio::io_service m_io_service;
    asio::steady_timer m_timer;
    bool m_time_expired = false;

};

class RTPSSubListener: public fastrtps::SubscriberListener
{
public:
    RTPSSubListener() : n_matched(0), n_msg(0), m_new_message(false){};
    ~RTPSSubListener(){};
    virtual void onSubscriptionMatched(eprosima::fastrtps::rtps::MatchingInfo& info) = 0;
    virtual void onNewDataMessage(fastrtps::Subscriber* sub) = 0;
    fastrtps::SampleInfo_t m_info;
    int n_matched;
    int n_msg;
    bool m_new_message;
};

/**
 * Class DataReader, contains the public API that allows the user to control the reception of messages.
 * @ingroup MICRORTPS_MODULE
 */
class DataReader: public XRCEObject, public TimerEvent, public RTPSSubListener
{

public:
    DataReader(eprosima::fastrtps::Participant* rtps_participant, ReaderListener* read_list);
    virtual ~DataReader();

    bool init();
    int read(const READ_DATA_Payload& read_data);


    void on_timeout(const asio::error_code& error);
    void onSubscriptionMatched(eprosima::fastrtps::rtps::MatchingInfo& info);
    void onNewDataMessage(fastrtps::Subscriber* sub);


private:
    int start_read(const READ_DATA_Payload& read_data);
    int stop_read();
    void read_task(READ_DATA_Payload read_data);
    bool takeNextData(void* data);

    std::thread m_read_thread, m_timer_thread;
    std::mutex m_mutex;
    std::condition_variable m_cond_var;
    std::atomic_bool m_running;

    ReaderListener* mp_reader_listener;
    std::string m_rtps_subscriber_prof;
    eprosima::fastrtps::Participant* mp_rtps_participant;
    eprosima::fastrtps::Subscriber* mp_rtps_subscriber;
    ShapeTypePubSubType m_shape_type;
};



} /* namespace micrortps */
} /* namespace eprosima */

#endif /* DATAREADER_H_ */
