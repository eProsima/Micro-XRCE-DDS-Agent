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

#ifndef UXR_AGENT_READER_READER_HPP_
#define UXR_AGENT_READER_READER_HPP_

#include <uxr/agent/types/XRCETypes.hpp>
#include <uxr/agent/utils/TokenBucket.hpp>

#include <atomic>
#include <thread>
#include <mutex>
#include <chrono>
#include <type_traits>

namespace eprosima {
namespace uxr {

class ProxyClient;

struct WriteFnArgs
{
    std::shared_ptr<ProxyClient> client;
    dds::xrce::ClientKey client_key;
    dds::xrce::StreamId stream_id;
    dds::xrce::ObjectId object_id;
    dds::xrce::RequestId request_id;
};

template<typename RA, typename WA = const WriteFnArgs&>
class Reader
{
public:
    typedef const std::function<bool (RA, std::vector<uint8_t>&, std::chrono::milliseconds)> ReadFn;
    typedef const std::function<bool (WA, const std::vector<uint8_t>&, std::chrono::milliseconds)> WriteFn;

public:
    ~Reader();

    bool start_reading(
        const dds::xrce::DataDeliveryControl& delivery_control,
        ReadFn read_fn,
        RA read_args,
        WriteFn write_fn,
        WA write_args);

    bool stop_reading();

private:
    void read_task(
        ReadFn read_fn,
        WriteFn write_fn);

private:
    dds::xrce::DataDeliveryControl delivery_control_;
    typename std::decay<RA>::type read_args_;
    typename std::decay<WA>::type write_args_;
    std::atomic<bool> running_cond_;
    std::thread thread_;
    std::mutex mtx_;

    static constexpr uint8_t rw_timeout = 100;
    static constexpr uint16_t max_samples_zero = 0;
    static constexpr uint16_t max_samples_unlimited = 0xFFFF;
    static constexpr uint16_t max_elapsed_time_unlimited = 0;
    static constexpr uint16_t max_bytes_per_second_unlimited = 0;
};

template<typename RA, typename WA>
inline Reader<RA, WA>::~Reader()
{
    stop_reading();
}

template<typename RA, typename WA>
inline bool Reader<RA, WA>::start_reading(
        const dds::xrce::DataDeliveryControl& delivery_control,
        ReadFn read_fn,
        RA read_args,
        WriteFn write_fn,
        WA write_args)
{
    std::lock_guard<std::mutex> lock(mtx_);
    bool rv = false;
    if (!running_cond_)
    {
        delivery_control_ = delivery_control;
        read_args_ = read_args;
        write_args_ = write_args;
        running_cond_ = true;
        thread_ = std::thread(&Reader<RA, WA>::read_task, this, read_fn, write_fn);
        rv = true;
    }
    return rv;
}

template<typename RA, typename WA>
inline bool Reader<RA, WA>::stop_reading()
{
    std::lock_guard<std::mutex> lock(mtx_);
    bool rv = true;
    if (running_cond_)
    {
        running_cond_ = false;
        if (thread_.joinable())
        {
            thread_.join();
        }
        else
        {
            rv = false;
        }
    }
    return rv;
}

template<typename RA, typename WA>
inline void Reader<RA, WA>::read_task(
        ReadFn read_fn,
        WriteFn write_fn)
{
    using namespace eprosima::uxr::utils;
    using namespace std::chrono;

    constexpr std::chrono::milliseconds max_timeout{rw_timeout};

    size_t rate = (max_bytes_per_second_unlimited == delivery_control_.max_bytes_per_second())
        ? SIZE_MAX
        : delivery_control_.max_bytes_per_second();
    TokenBucket token_bucket{rate};
    bool stop_cond = false;
    uint16_t message_count = 0;
    std::vector<uint8_t> data;
    time_point<steady_clock> init_time = steady_clock::now();
    time_point<steady_clock> final_time = (max_elapsed_time_unlimited == delivery_control_.max_elapsed_time()) 
        ? time_point<steady_clock>::max()
        : init_time + seconds(delivery_control_.max_elapsed_time());

    milliseconds timeout;
    while (running_cond_ && !stop_cond)
    {
        timeout = std::min(max_timeout, duration_cast<milliseconds>(final_time - steady_clock::now()));
        if (read_fn(read_args_, data, timeout))
        {
            bool submessage_pushed = false;
            do {
                if (token_bucket.consume_tokens(data.size(), timeout))
                {
                    do {
                        timeout = std::min(max_timeout, duration_cast<milliseconds>(final_time - steady_clock::now()));
                        submessage_pushed = write_fn(write_args_, data, timeout);
                    } while (running_cond_ && !submessage_pushed);

                    if (submessage_pushed)
                    {
                        ++message_count;
                    }
                }
            } while(running_cond_ && !submessage_pushed);
        }

        stop_cond = ((max_samples_unlimited != delivery_control_.max_samples()) &&
                     (message_count == delivery_control_.max_samples())) || 
                    (std::chrono::steady_clock::now() > final_time);
    }
}

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_READER_READER_HPP_