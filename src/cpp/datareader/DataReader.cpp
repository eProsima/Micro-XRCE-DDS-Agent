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

#include <uxr/agent/datareader/DataReader.hpp>
#include <uxr/agent/subscriber/Subscriber.hpp>
#include <uxr/agent/participant/Participant.hpp>
#include <uxr/agent/topic/Topic.hpp>
#include <uxr/agent/middleware/Middleware.hpp>
#include <uxr/agent/utils/TokenBucket.hpp>
#include <uxr/agent/logger/Logger.hpp>

#include <iostream>
#include <atomic>

namespace eprosima {
namespace uxr {

using utils::TokenBucket;

std::unique_ptr<DataReader> DataReader::create(const dds::xrce::ObjectId& object_id,
        const std::shared_ptr<Subscriber>& subscriber,
        const dds::xrce::DATAREADER_Representation& representation,
        const ObjectContainer& root_objects)
{
    bool created_entity = false;
    uint16_t raw_object_id = conversion::objectid_to_raw(object_id);
    std::shared_ptr<Topic> topic;

    Middleware& middleware = subscriber->get_middleware();
    switch (representation.representation()._d())
    {
        case dds::xrce::REPRESENTATION_BY_REFERENCE:
        {
            const std::string& ref = representation.representation().object_reference();
            uint16_t raw_topic_id;
            if (middleware.create_datareader_by_ref(raw_object_id, subscriber->get_raw_id(), ref, raw_topic_id))
            {
                dds::xrce::ObjectId topic_id = conversion::raw_to_objectid(raw_topic_id, dds::xrce::OBJK_TOPIC);;
                topic = std::dynamic_pointer_cast<Topic>(root_objects.at(topic_id));
                topic->tie_object(object_id);
                created_entity = true;
            }
            break;
        }
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml = representation.representation().xml_string_representation();
            uint16_t raw_topic_id;
            if (middleware.create_datareader_by_xml(raw_object_id, subscriber->get_raw_id(), xml, raw_topic_id))
            {
                dds::xrce::ObjectId topic_id = conversion::raw_to_objectid(raw_topic_id, dds::xrce::OBJK_TOPIC);
                topic = std::dynamic_pointer_cast<Topic>(root_objects.at(topic_id));
                topic->tie_object(object_id);
                created_entity = true;
            }
            break;
        }
        default:
            break;
    }

    return (created_entity ? std::unique_ptr<DataReader>(new DataReader(object_id, subscriber, topic)) : nullptr);
}

DataReader::DataReader(const dds::xrce::ObjectId& object_id,
        const std::shared_ptr<Subscriber>& subscriber,
        const std::shared_ptr<Topic>& topic)
    : XRCEObject(object_id)
    , subscriber_(subscriber)
    , topic_(topic)
    , running_cond_(false)
{
    subscriber_->tie_object(object_id);
    topic_->tie_object(object_id);
}

DataReader::~DataReader() noexcept
{
    stop_read();
    subscriber_->untie_object(get_id());
    topic_->untie_object(get_id());
    get_middleware().delete_datareader(get_raw_id());
}

bool DataReader::read(const dds::xrce::READ_DATA_Payload& read_data,
                      read_callback read_cb,
                      const ReadCallbackArgs& cb_args)
{
    dds::xrce::DataDeliveryControl delivery_control;
    if (read_data.read_specification().has_delivery_control())
    {
        delivery_control = read_data.read_specification().delivery_control();
    }
    else
    {
        delivery_control.max_elapsed_time(0);
        delivery_control.max_bytes_per_second(0);
        delivery_control.max_samples(1);
    }

    switch (read_data.read_specification().data_format())
    {
        case dds::xrce::FORMAT_DATA:
            break;
        case dds::xrce::FORMAT_SAMPLE:
            break;
        case dds::xrce::FORMAT_DATA_SEQ:
            break;
        case dds::xrce::FORMAT_SAMPLE_SEQ:
            break;
        case dds::xrce::FORMAT_PACKED_SAMPLES:
            break;
        default:
            std::cout << "Error: read format unexpected" << std::endl;
            break;
    }

    return (stop_read() && start_read(delivery_control, read_cb, cb_args));
}

bool DataReader::start_read(const dds::xrce::DataDeliveryControl& delivery_control, read_callback read_cb, const ReadCallbackArgs& cb_args)
{
    std::lock_guard<std::mutex> lock(mtx_);
    running_cond_ = true;
    if (delivery_control.max_elapsed_time() > 0)
    {
        max_timer_thread_ = std::thread(&DataReader::run_max_timer, this, delivery_control.max_elapsed_time());
    }
    read_thread_ = std::thread(&DataReader::read_task, this, delivery_control, read_cb, cb_args);
    return true;
}

bool DataReader::stop_read()
{
    std::lock_guard<std::mutex> lock(mtx_);
    running_cond_ = false;

    if (read_thread_.joinable())
    {
        read_thread_.join();
    }
    stop_max_timer();
    if (max_timer_thread_.joinable())
    {
        max_timer_thread_.join();
    }
    return true;
}

void DataReader::read_task(dds::xrce::DataDeliveryControl delivery_control,
                           read_callback read_cb,
                           ReadCallbackArgs cb_args)
{
    TokenBucket rate_manager{delivery_control.max_bytes_per_second()};
    uint16_t message_count = 0;

    while (running_cond_ && (message_count < delivery_control.max_samples()))
    {
        std::vector<uint8_t> data;
        if (get_middleware().read_data(get_raw_id(), data, std::chrono::milliseconds(100)))
        {
            while (!rate_manager.get_tokens(data.size()))
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            UXR_AGENT_LOG_MESSAGE(
                data.data(),
                data.size(),
                UXR_DECORATE_YELLOW("[==>> DDS <<==]"),
            get_raw_id());
            read_cb(cb_args, data);
            ++message_count;
        }
    }
}

void DataReader::on_max_timeout(const asio::error_code& error)
{
    if (!error)
    {
        running_cond_ = false;
    }
}

bool DataReader::matched(const dds::xrce::ObjectVariant& new_object_rep) const
{
    /* Check ObjectKind. */
    if ((get_id().at(1) & 0x0F) != new_object_rep._d())
    {
        return false;
    }

    bool rv = false;
    switch (new_object_rep.data_reader().representation()._d())
    {
        case dds::xrce::REPRESENTATION_BY_REFERENCE:
        {
            const std::string& ref = new_object_rep.data_reader().representation().object_reference();
            rv = get_middleware().matched_datareader_from_ref(get_raw_id(), ref);
            break;
        }
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml = new_object_rep.data_reader().representation().xml_string_representation();
            rv = get_middleware().matched_datareader_from_xml(get_raw_id(), xml);
            break;
        }
        default:
            break;
    }
    return rv;
}

Middleware& DataReader::get_middleware() const
{
    return subscriber_->get_middleware();
}

ReadTimeEvent::ReadTimeEvent()
    : m_timer_max(m_io_service_max)
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
    init_max_timer(milliseconds);
    m_io_service_max.run();
}

} // namespace uxr
} // namespace eprosima
