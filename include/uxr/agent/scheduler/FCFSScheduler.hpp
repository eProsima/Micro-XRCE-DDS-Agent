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

#ifndef UXR_AGENT_SCHEDULER_FCFS_SCHEDULER_HPP_
#define UXR_AGENT_SCHEDULER_FCFS_SCHEDULER_HPP_

#include <uxr/agent/scheduler/Scheduler.hpp>

#include <deque>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace eprosima {
namespace uxr {

template<class T>
class FCFSScheduler : public Scheduler<T>
{
public:
    FCFSScheduler(
            size_t max_size)
        : deque_()
        , mtx_()
        , cond_var_()
        , running_cond_(false)
        , max_size_{max_size}
    {}

    void init() final;

    void deinit() final;

    void push(
            T&& element,
            uint8_t priority) final;

    void push_front(
            T&& element);

    bool pop(
            T& element) final;

private:
    std::deque<T> deque_;
    std::mutex mtx_;
    std::condition_variable cond_var_;
    bool running_cond_;
    const size_t max_size_;
};

template<class T>
inline void FCFSScheduler<T>::init()
{
    std::lock_guard<std::mutex> lock(mtx_);
    running_cond_ = true;
}

template<class T>
inline void FCFSScheduler<T>::deinit()
{
    std::lock_guard<std::mutex> lock(mtx_);
    running_cond_ = false;
    cond_var_.notify_one();
}

template<class T>
inline void FCFSScheduler<T>::push(
        T&& element,
        uint8_t priority)
{
    (void) priority;
    std::lock_guard<std::mutex> lock(mtx_);
    if (max_size_ <= deque_.size())
    {
        deque_.pop_front();
    }
    deque_.push_back(std::move(element));
    cond_var_.notify_one();
}

template<class T>
inline void FCFSScheduler<T>::push_front(
        T&& element)
{
    std::lock_guard<std::mutex> lock(mtx_);
    deque_.push_front(std::forward<T>(element));
}

template<class T>
inline bool FCFSScheduler<T>::pop(
        T& element)
{
    bool rv = false;
    std::unique_lock<std::mutex> lock(mtx_);
    cond_var_.wait(lock, [this] { return !(deque_.empty() && running_cond_); });
    if (running_cond_)
    {
        element = std::move(deque_.front());
        deque_.pop_front();
        rv = true;
        cond_var_.notify_one();
    }
    return rv;
}

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_SCHEDULER_FCFS_SCHEDULER_HPP_
