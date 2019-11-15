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

#ifndef UXR_AGENT_DATAREADER_DATAREADER_HPP_
#define UXR_AGENT_DATAREADER_DATAREADER_HPP_

#include <uxr/agent/object/XRCEObject.hpp>

#include <condition_variable>
#include <mutex>
#include <thread>
#include <functional>
#include <atomic>

namespace eprosima {
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

typedef const std::function<bool (const ReadCallbackArgs&, std::vector<uint8_t>, std::chrono::milliseconds)> read_callback;

/**
 * @brief The DataReader class
 */
class DataReader : public XRCEObject
{
public:
    static std::unique_ptr<DataReader> create(
        const dds::xrce::ObjectId& object_id,
        const std::shared_ptr<Subscriber>& subscriber,
        const dds::xrce::DATAREADER_Representation& representation,
        const ObjectContainer& root_objects);

    virtual ~DataReader() noexcept override;

    DataReader(DataReader&&) = delete;
    DataReader(const DataReader&) = delete;
    DataReader& operator=(DataReader&&) = delete;
    DataReader& operator=(const DataReader&) = delete;

    void release(
            ObjectContainer&) override {}

    bool matched(
            const dds::xrce::ObjectVariant& new_object_rep) const override;

    Middleware& get_middleware() const override;

    bool read(
        const dds::xrce::READ_DATA_Payload& read_data,
        read_callback read_cb,
        const ReadCallbackArgs& cb_args);

private:
    DataReader(
        const dds::xrce::ObjectId& object_id,
        const std::shared_ptr<Subscriber>& subscriber,
        const std::shared_ptr<Topic>& topic);

    bool start_read(
        const dds::xrce::DataDeliveryControl& delivery_control,
        read_callback read_cb,
        const ReadCallbackArgs& cb_args);

    bool stop_read();

    void read_task(
        dds::xrce::DataDeliveryControl delivery_control,
        read_callback read_cb,
        ReadCallbackArgs cb_args);

private:
    std::shared_ptr<Subscriber> subscriber_;
    std::shared_ptr<Topic> topic_;
    std::atomic<bool> running_cond_;
    std::thread read_thread_;
    std::mutex mtx_;
};


} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_DATAREADER_DATAREADER_HPP_
