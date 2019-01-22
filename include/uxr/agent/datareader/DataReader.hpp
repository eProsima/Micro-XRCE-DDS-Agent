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

#ifndef _UXR_AGENT_DATAREADER_DATAREADER_HPP_
#define _UXR_AGENT_DATAREADER_DATAREADER_HPP_

#include <uxr/agent/object/XRCEObject.hpp>
#include <uxr/agent/types/TopicPubSubType.hpp>
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

namespace uxr {

class Subscriber;
class Topic;
class Middleware;

/**
 * Callback data structure.
 */
struct ReadCallbackArgs
{
    dds::xrce::ClientKey client_key;
    dds::xrce::StreamId stream_id;
    dds::xrce::ObjectId object_id;
    dds::xrce::RequestId request_id;

};

typedef const std::function<void (const ReadCallbackArgs&, std::vector<uint8_t>)> read_callback;

/**
 * @brief The ReadTimeEvent class
 */
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

/**
 * @brief The RTPSSubListener class
 */
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

  private:
    using fastrtps::SubscriberListener::onSubscriptionMatched;
};

/**
 * @brief The DataReader class contains the public API that allows the user to control the reception of message.
 */
class DataReader : public XRCEObject, public ReadTimeEvent, public RTPSSubListener
{
public:
    DataReader(const dds::xrce::ObjectId& object_id,
               Middleware* middleware,
               const std::shared_ptr<Subscriber>& subscriber,
               const std::string& profile_name = "");

    virtual ~DataReader() noexcept override;

    DataReader(DataReader&&)      = delete;
    DataReader(const DataReader&) = delete;
    DataReader& operator=(DataReader&&) = delete;
    DataReader& operator=(const DataReader&) = delete;

    bool init_middleware(const dds::xrce::DATAREADER_Representation& representation,
            const ObjectContainer& root_objects);

    void read(const dds::xrce::READ_DATA_Payload& read_data, read_callback read_cb, const ReadCallbackArgs& cb_args);
    void on_max_timeout(const asio::error_code& error) override;
    void onSubscriptionMatched(fastrtps::Subscriber* sub, fastrtps::rtps::MatchingInfo& info) override;
    void onNewDataMessage(fastrtps::Subscriber*) override;
    void on_new_message();
    void release(ObjectContainer&) override {}
    bool matched(const dds::xrce::ObjectVariant& new_object_rep) const override;

private:
    int start_read(const dds::xrce::DataDeliveryControl& delivery_control,
                   read_callback read_cb,
                   const ReadCallbackArgs& cb_args);

    int stop_read();

    void read_task(dds::xrce::DataDeliveryControl delivery_control,
                   read_callback read_cb,
                   ReadCallbackArgs cb_args);

    bool takeNextData(void* data);
    size_t nextDataSize();

private:
    std::shared_ptr<Subscriber> subscriber_;
    std::shared_ptr<Topic> topic_;
    std::thread read_thread_;
    std::thread max_timer_thread_;
    std::mutex mtx_;
    std::condition_variable cond_var_;
    bool running_cond_;
    std::string rtps_subscriber_prof_;
    fastrtps::Subscriber* rtps_subscriber_;
    TopicPubSubType topic_type_;
};

} // namespace uxr
} // namespace eprosima

#endif //_UXR_AGENT_DATAREADER_DATAREADER_HPP_
