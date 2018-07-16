// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _MICRORTPS_AGENT_DATAREADER_DATAREADER_HPP_
#define _MICRORTPS_AGENT_DATAREADER_DATAREADER_HPP_

#include <micrortps/agent/object/XRCEObject.hpp>
#include <micrortps/agent/types/TopicPubSubType.hpp>

#include <fastrtps/subscriber/SampleInfo.h>
#include <fastrtps/subscriber/SubscriberListener.h>

#include <asio/io_service.hpp>
#include <asio/steady_timer.hpp>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <functional>

namespace eprosima {

namespace fastrtps {
class Participant;
class Subscriber;
namespace rtps {
class MatchingInfo;
} // namespace rtps
} // namespace fastrtps

namespace micrortps {

class Processor;

typedef struct ReadCallbackArgs
{
    dds::xrce::ClientKey client_key;
    dds::xrce::StreamId stream_id;
    dds::xrce::ObjectId object_id;
    dds::xrce::RequestId request_id;

} ReadCallbackArgs;

typedef const std::function<void (const ReadCallbackArgs&, std::vector<uint8_t>)> read_callback;

class ReaderListener
{
  public:
    ReaderListener()          = default;
    virtual ~ReaderListener() = default;

    ReaderListener(ReaderListener&&)      = default;
    ReaderListener(const ReaderListener&) = default;
    ReaderListener& operator=(ReaderListener&&) = default;
    ReaderListener& operator=(const ReaderListener&) = default;

    virtual void on_read_data(const dds::xrce::StreamId& stream_id,
                              const dds::xrce::ObjectId& object_id,
                              const dds::xrce::RequestId& request_id,
                              const std::vector<unsigned char>& buffer) = 0;
};

class ReadTimeEvent
{
  public:
    ReadTimeEvent();
    virtual ~ReadTimeEvent()            = default;
    ReadTimeEvent(ReadTimeEvent&&)      = delete;
    ReadTimeEvent(const ReadTimeEvent&) = delete;
    ReadTimeEvent& operator=(ReadTimeEvent&&) = delete;
    ReadTimeEvent& operator=(const ReadTimeEvent&) = delete;

    int init_max_timer(int milliseconds);
    void stop_max_timer();
    void run_max_timer(int milliseconds);

    virtual void on_max_timeout(const asio::error_code& error) = 0;

  protected:
    asio::io_service m_io_service_max;
    asio::steady_timer m_timer_max;
};

class RTPSSubListener : public fastrtps::SubscriberListener
{
  public:
    RTPSSubListener()           = default;
    ~RTPSSubListener() override = default;

    RTPSSubListener(RTPSSubListener&&)      = delete;
    RTPSSubListener(const RTPSSubListener&) = delete;
    RTPSSubListener& operator=(RTPSSubListener&&) = delete;
    RTPSSubListener& operator=(const RTPSSubListener&) = delete;

    void onSubscriptionMatched(fastrtps::Subscriber* sub, fastrtps::rtps::MatchingInfo& info) override = 0;
    void onNewDataMessage(fastrtps::Subscriber* sub) override                                          = 0;
    fastrtps::SampleInfo_t info_;
    int matched_{0};
    std::atomic_bool msg_{false};

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
    DataReader(const dds::xrce::ObjectId& id,
               eprosima::fastrtps::Participant& rtps_participant,
               ReaderListener* reader_listener,
               const std::string& profile_name = "");
    ~DataReader() noexcept override;

    DataReader(DataReader&&)      = delete;
    DataReader(const DataReader&) = delete;
    DataReader& operator=(DataReader&&) = delete;
    DataReader& operator=(const DataReader&) = delete;

    bool init();
    bool init(const std::string& xmlrep);
    void read(const dds::xrce::READ_DATA_Payload& read_data, read_callback read_cb, const ReadCallbackArgs& cb_args);
    bool has_message() const;

    void on_max_timeout(const asio::error_code& error) override;

    void onSubscriptionMatched(eprosima::fastrtps::Subscriber* sub,
                               eprosima::fastrtps::rtps::MatchingInfo& info) override;
    void onNewDataMessage(fastrtps::Subscriber*) override;

private:
    int start_read(const dds::xrce::DataDeliveryControl& delivery_control,
                   read_callback read_cb, const ReadCallbackArgs& cb_args);
    int stop_read();
    void read_task(dds::xrce::DataDeliveryControl delivery_control,
                   read_callback read_cb, ReadCallbackArgs cb_args);
    bool takeNextData(void* data);
    size_t nextDataSize();
    bool check_registered_topic(const std::string& topic_data_type) const;

    std::thread m_read_thread;
    std::thread m_max_timer_thread;
    std::mutex m_mutex;
    std::condition_variable m_cond_var;
    bool m_running;

    ReaderListener* mp_reader_listener;
    std::string m_rtps_subscriber_prof;
    fastrtps::Participant& mp_rtps_participant;
    fastrtps::Subscriber* mp_rtps_subscriber;
    TopicPubSubType topic_type_;
};

} // namespace micrortps
} // namespace eprosima

#endif //_MICRORTPS_AGENT_DATAREADER_DATAREADER_HPP_
