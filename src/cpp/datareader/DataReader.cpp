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

constexpr int READ_TIMEOUT = 100;
constexpr uint8_t PUSH_SUBMESSAGE_TIMEOUT = 100;
constexpr std::chrono::milliseconds MAX_SLEEP_TIME{100};
constexpr uint16_t MAX_SAMPLES_ZERO = 0;
constexpr uint16_t MAX_SAMPLES_UNLIMITED = 0xFFFF;
constexpr uint16_t MAX_ELAPSED_TIME_UNLIMITED = 0;
constexpr uint16_t MAX_BYTES_PER_SECOND_UNLIMITED = 0;

namespace  {

std::chrono::milliseconds get_read_timeout(
        const std::chrono::steady_clock::time_point& final_time,
        uint16_t max_elapsed_time)
{
    std::chrono::milliseconds rv = std::chrono::milliseconds(READ_TIMEOUT);
    if (MAX_ELAPSED_TIME_UNLIMITED != max_elapsed_time)
    {
        rv = std::min(
             std::chrono::milliseconds(READ_TIMEOUT),
             std::chrono::duration_cast<std::chrono::milliseconds>(final_time - std::chrono::steady_clock::now()));
    }
    return rv;
}

} // unnamed namespace

using utils::TokenBucket;

std::unique_ptr<DataReader> DataReader::create(
        const dds::xrce::ObjectId& object_id,
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

DataReader::DataReader(
        const dds::xrce::ObjectId& object_id,
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

bool DataReader::matched(
        const dds::xrce::ObjectVariant& new_object_rep) const
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

bool DataReader::read(
        const dds::xrce::READ_DATA_Payload& read_data,
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

bool DataReader::start_read(
        const dds::xrce::DataDeliveryControl& delivery_control,
        read_callback read_cb,
        const ReadCallbackArgs& cb_args)
{
    std::lock_guard<std::mutex> lock(mtx_);
    running_cond_ = true;
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
    return true;
}

void DataReader::read_task(
        dds::xrce::DataDeliveryControl delivery_control,
        read_callback read_cb,
        ReadCallbackArgs cb_args)
{
    size_t rate = (MAX_BYTES_PER_SECOND_UNLIMITED == delivery_control.max_bytes_per_second())
            ? SIZE_MAX
            : delivery_control.max_bytes_per_second();
    TokenBucket token_bucket{rate};
    bool stop_cond = false;
    uint16_t message_count = 0;
    std::chrono::time_point<std::chrono::steady_clock> init_time
            = std::chrono::steady_clock::now();
    const std::chrono::time_point<std::chrono::steady_clock> final_time
            = init_time + std::chrono::seconds(delivery_control.max_elapsed_time());
    std::vector<uint8_t> data;
    while (running_cond_ && !stop_cond)
    {
        std::chrono::milliseconds read_timeout = get_read_timeout(final_time, delivery_control.max_elapsed_time());
        if (get_middleware().read_data(get_raw_id(), data, read_timeout))
        {
            bool submessage_pushed = false;
            do
            {
                if (token_bucket.consume_tokens(data.size(), MAX_SLEEP_TIME))
                {
                    do {
                        submessage_pushed = read_cb(cb_args, data, std::chrono::milliseconds(PUSH_SUBMESSAGE_TIMEOUT));
                    } while (running_cond_ && !submessage_pushed);

                    if (submessage_pushed)
                    {
                        UXR_AGENT_LOG_MESSAGE(
                            UXR_DECORATE_YELLOW("[==>> DDS <<==]"),
                            get_raw_id(),
                            data.data(),
                            data.size());
                        ++message_count;
                    }
                }
            } while (running_cond_ && !submessage_pushed);
        }

        stop_cond = ((MAX_SAMPLES_UNLIMITED != delivery_control.max_samples()) &&
                     (message_count == delivery_control.max_samples())) ||
                    ((MAX_ELAPSED_TIME_UNLIMITED != delivery_control.max_elapsed_time()) &&
                     (std::chrono::steady_clock::now() > final_time));
    }
}

} // namespace uxr
} // namespace eprosima
