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

#include <fastrtps/Domain.h>
#include <fastrtps/subscriber/SampleInfo.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>
#include <atomic>
#include "../xmlobjects/xmlobjects.h"

namespace eprosima {
namespace uxr {

using utils::TokenBucket;

DataReader::DataReader(const dds::xrce::ObjectId& object_id,
                       const std::shared_ptr<Subscriber>& subscriber,
                       const std::string& profile_name)
    : XRCEObject(object_id),
      subscriber_(subscriber),
      running_cond_(false),
      rtps_subscriber_prof_(profile_name),
      rtps_subscriber_(nullptr),
      topic_type_(false)
{
    subscriber_->tie_object(object_id);
}

DataReader::~DataReader() noexcept
{
    stop_read();
    if (read_thread_.joinable())
    {
        read_thread_.join();
    }

    if (max_timer_thread_.joinable())
    {
        max_timer_thread_.join();
    }

    if (nullptr != rtps_subscriber_)
    {
        fastrtps::Domain::removeSubscriber(rtps_subscriber_);
    }

    subscriber_->untie_object(get_id());
    if (topic_)
    {
        topic_->untie_object(get_id());
    }
}

bool DataReader::init_middleware(
        Middleware *middleware,
        const dds::xrce::DATAREADER_Representation &representation,
        const ObjectContainer &root_objects)
{
    bool rv = false;
    switch (representation.representation()._d())
    {
        case dds::xrce::REPRESENTATION_BY_REFERENCE:
        {
            const std::string& ref = representation.representation().object_reference();
            uint16_t topic_id;
            if (middleware->create_datareader_from_ref(
                        get_raw_id(),
                        subscriber_->get_raw_id(),
                        ref,
                        topic_id,
                        std::bind(&DataReader::on_new_message, this)))
            {
                dds::xrce::ObjectId topic_xrce_id = {uint8_t(topic_id >> 8), uint8_t(topic_id & 0xFF)};
                topic_ = std::dynamic_pointer_cast<Topic>(root_objects.at(topic_xrce_id));
                topic_->tie_object(get_id());
                rv = true;
            }
            break;
        }
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml = representation.representation().xml_string_representation();
            uint16_t topic_id;
            if (middleware->create_datareader_from_xml(
                        get_raw_id(),
                        subscriber_->get_raw_id(),
                        xml,
                        topic_id,
                        std::bind(&DataReader::on_new_message, this)))
            {
                dds::xrce::ObjectId topic_xrce_id = {uint8_t(topic_id >> 8), uint8_t(topic_id & 0xFF)};
                topic_ = std::dynamic_pointer_cast<Topic>(root_objects.at(topic_xrce_id));
                topic_->tie_object(get_id());
                rv = true;
            }
            break;
        }
        default:
            break;
    }
    return rv;
}

void DataReader::read(const dds::xrce::READ_DATA_Payload& read_data,
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

    stop_read();
    start_read(delivery_control, read_cb, cb_args);
}

int DataReader::start_read(const dds::xrce::DataDeliveryControl& delivery_control, read_callback read_cb, const ReadCallbackArgs& cb_args)
{
    std::unique_lock<std::mutex> lock(mtx_);
    running_cond_ = true;
    lock.unlock();

    if (delivery_control.max_elapsed_time() > 0)
    {
        max_timer_thread_ = std::thread(&DataReader::run_max_timer, this, delivery_control.max_elapsed_time());
    }
    read_thread_ = std::thread(&DataReader::read_task, this, delivery_control, read_cb, cb_args);

    return 0;
}

int DataReader::stop_read()
{
    std::unique_lock<std::mutex> lock(mtx_);
    running_cond_ = false;
    lock.unlock();
    cond_var_.notify_one();

    if (read_thread_.joinable())
    {
        read_thread_.join();
    }

    stop_max_timer();
    if (max_timer_thread_.joinable())
    {
        max_timer_thread_.join();
    }
    return 0;
}

void DataReader::read_task(dds::xrce::DataDeliveryControl delivery_control,
                           read_callback read_cb,
                           ReadCallbackArgs cb_args)
{
    TokenBucket rate_manager{delivery_control.max_bytes_per_second()};
    uint16_t message_count = 0;
    while (true)
    {
        std::unique_lock<std::mutex> lock(mtx_);
        if (running_cond_ && (message_count < delivery_control.max_samples()))
        {
            if (rtps_subscriber_->getUnreadCount() != 0)
            {
                lock.unlock();
                /* Read operation. */
                size_t next_data_size = nextDataSize();
                if (next_data_size != 0u && rate_manager.get_tokens(next_data_size))
                {
                    std::vector<unsigned char> buffer;
                    if (takeNextData(&buffer))
                    {
                        read_cb(cb_args, buffer);
                        ++message_count;
                    }
                }
            }
            else
            {
                /* Wait for new message or terminate signal. */
                cond_var_.wait(lock);
                lock.unlock();
            }
        }
        else
        {
            running_cond_ = false;
            lock.unlock();
            stop_max_timer();
            break;
        }
    }
}

void DataReader::on_max_timeout(const asio::error_code& error)
{
    if (!error)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        running_cond_ = false;
        cond_var_.notify_one();
    }
}

void DataReader::onNewDataMessage(eprosima::fastrtps::Subscriber* /*sub*/)
{
    std::lock_guard<std::mutex> lock(mtx_);
    cond_var_.notify_one();
}

void DataReader::on_new_message()
{
    std::lock_guard<std::mutex> lock(mtx_);
    cond_var_.notify_one();
}

bool DataReader::matched(const dds::xrce::ObjectVariant& new_object_rep) const
{
    /* Check ObjectKind. */
    if ((get_id().at(1) & 0x0F) != new_object_rep._d())
    {
        return false;
    }

    bool parser_cond = false;
    const fastrtps::SubscriberAttributes& old_attributes = rtps_subscriber_->getAttributes();
    fastrtps::SubscriberAttributes new_attributes;

    switch (new_object_rep.data_reader().representation()._d())
    {
        case dds::xrce::REPRESENTATION_BY_REFERENCE:
        {
            const std::string& ref_rep = new_object_rep.data_reader().representation().object_reference();
            if (fastrtps::xmlparser::XMLP_ret::XML_OK ==
                fastrtps::xmlparser::XMLProfileManager::fillSubscriberAttributes(ref_rep, new_attributes))
            {
                parser_cond = true;
            }
            break;
        }
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml_rep = new_object_rep.data_reader().representation().xml_string_representation();
            if (xmlobjects::parse_subscriber(xml_rep.data(), xml_rep.size(), new_attributes))
            {
                parser_cond = true;
            }
            break;
        }
        default:
            break;
    }

    return parser_cond && (new_attributes == old_attributes);
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

bool DataReader::takeNextData(void* data)
{
    if (nullptr == rtps_subscriber_)
    {
        return false;
    }
    fastrtps::SampleInfo_t info;
    return rtps_subscriber_->takeNextData(data, &info);
}

size_t DataReader::nextDataSize()
{
    std::vector<unsigned char> buffer;
    fastrtps::SampleInfo_t info;
    // TODO (Borja): review KEE_PALL configuration.
    if (rtps_subscriber_->readNextData(&buffer, &info))
    {
        if (info_.sampleKind == rtps::ALIVE)
        {
            return buffer.size();
        }
    }
    return 0;
}

void DataReader::onSubscriptionMatched(fastrtps::Subscriber* /*sub*/, fastrtps::rtps::MatchingInfo& info)
{
    if (info.status == rtps::MATCHED_MATCHING)
    {
        std::cout << "RTPS Publisher matched " << info.remoteEndpointGuid << std::endl;
    }
    else
    {
        std::cout << "RTPS Publisher unmatched " << info.remoteEndpointGuid << std::endl;
    }
}

} // namespace uxr
} // namespace eprosima
