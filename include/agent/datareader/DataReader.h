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

#include <asio/io_service.hpp>
#include <asio/steady_timer.hpp>
#include <condition_variable>
#include <mutex>
#include <thread>

#include <fastrtps/rtps/common/MatchingInfo.h>
#include <fastrtps/subscriber/SampleInfo.h>
#include <fastrtps/subscriber/SubscriberListener.h>

#include <DDSXRCETypes.h>
#include <Payloads.h>
#include <agent/Common.h>
#include <agent/types/ShapePubSubTypes.h>

namespace eprosima {

namespace fastrtps {
class Participant;
class Subscriber;
} // namespace fastrtps

namespace micrortps {

class ReaderListener
{
  public:
    ReaderListener()          = default;
    virtual ~ReaderListener() = default;

    virtual void on_read_data(const ObjectId& object_id, const RequestId& req_id,
                              const std::vector<unsigned char>& buffer) = 0;
};

class ReadTimeEvent
{
  public:
    ReadTimeEvent();
    virtual ~ReadTimeEvent() = default;
    int init_max_timer(int milliseconds);
    int init_rate_timer(int milliseconds);
    void run_max_timer(int milliseconds);
    void run_rate_timer(int milliseconds);

    virtual void on_max_timeout(const asio::error_code& error)  = 0;
    virtual void on_rate_timeout(const asio::error_code& error) = 0;

  protected:
    asio::io_service m_io_service_max;
    asio::io_service m_io_service_rate;
    asio::steady_timer m_timer_max;
    asio::steady_timer m_timer_rate;
    std::atomic<bool> m_max_time_expired;
  };

class RTPSSubListener : public fastrtps::SubscriberListener
{
  public:
    RTPSSubListener() : n_matched(0), n_msg(0){};
    virtual ~RTPSSubListener()                                                       = default;
    virtual void onSubscriptionMatched(eprosima::fastrtps::rtps::MatchingInfo& info) = 0;
    virtual void onNewDataMessage(fastrtps::Subscriber* sub)                         = 0;
    fastrtps::SampleInfo_t m_info;
    int n_matched;
    int n_msg;

  private:
    using fastrtps::SubscriberListener::onSubscriptionMatched;
};

/**
 * Class DataReader, contains the public API that allows the user to control the reception of messages.
 * @ingroup MICRORTPS_MODULE
 */
class DataReader : public XRCEObject, public ReadTimeEvent, public RTPSSubListener
{

  public:
    DataReader(eprosima::fastrtps::Participant* rtps_participant, ReaderListener* read_list);
    ~DataReader() noexcept override;

    bool init();
    bool init(const std::string& xmlrep);
    int read(const READ_DATA_Payload& read_data);

    void on_max_timeout(const asio::error_code& error);
    void on_rate_timeout(const asio::error_code& error);
    void onSubscriptionMatched(eprosima::fastrtps::rtps::MatchingInfo& info);
    void onNewDataMessage(fastrtps::Subscriber* sub);

  private:
    struct ReadTaskInfo
    {
        ObjectId object_ID_;
        RequestId request_ID_;
        uint16_t max_samples_;      // Maximum number of samples
        uint32_t max_elapsed_time_; // In milliseconds
        uint32_t max_rate_;         // Bytes per second
    };

    int start_read(const ReadTaskInfo& read_info);
    int stop_read();
    void read_task(const ReadTaskInfo& read_info);
    bool takeNextData(void* data);

    std::thread m_read_thread;
    std::thread m_max_timer_thread;
    std::thread m_rate_timer_thread;
    std::mutex m_mutex;
    std::condition_variable m_cond_var;
    std::atomic<bool> m_running;

    ReaderListener* mp_reader_listener;
    std::string m_rtps_subscriber_prof;
    fastrtps::Participant* mp_rtps_participant;
    fastrtps::Subscriber* mp_rtps_subscriber;
    ShapeTypePubSubType m_shape_type;
};

} /* namespace micrortps */
} /* namespace eprosima */

#endif /* DATAREADER_H_ */
