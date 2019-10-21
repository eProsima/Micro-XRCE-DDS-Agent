// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef UXR_UTILS_SHAREDMUTEX_HPP_
#define UXR_UTILS_SHAREDMUTEX_HPP_

#include <thread>
#include <mutex>
#include <condition_variable>

namespace eprosima {
namespace uxr {
namespace utils {

class SharedMutex
{
public:
    friend class SharedLock;
    friend class ExclusiveLock;

    SharedMutex()
        : mtx_{}
        , cv_{}
        , shared_counter_{0}
        , exclusively_locked_{false}
    {}

private:
    std::mutex mtx_;
    std::condition_variable cv_;
    size_t shared_counter_;
    bool exclusively_locked_;
    std::thread::id exclusive_thread_id_;
};

class SharedLock
{
public:
    SharedLock(
            SharedMutex& shared_mtx)
        : shared_mtx_{shared_mtx}
        , locked_{false}
    {
        lock();
    }

    ~SharedLock()
    {
        unlock();
    }

    void lock()
    {
        if (!locked_)
        {
            std::thread::id thread_id = std::this_thread::get_id();
            std::unique_lock<std::mutex> lock{shared_mtx_.mtx_};
            shared_mtx_.cv_.wait(
                lock,
                [&]{ return !shared_mtx_.exclusively_locked_ || thread_id == shared_mtx_.exclusive_thread_id_; });
            ++shared_mtx_.shared_counter_;
            locked_ = true;
        }
    }

    void unlock()
    {
        if (locked_)
        {
            std::unique_lock<std::mutex> lock{shared_mtx_.mtx_};
            --shared_mtx_.shared_counter_;
            if (0 == shared_mtx_.shared_counter_)
            {
                shared_mtx_.cv_.notify_one();
            }
            locked_ = false;
        }
    }

private:
    SharedMutex& shared_mtx_;
    bool locked_;
};

class ExclusiveLock
{
public:
    ExclusiveLock(
            SharedMutex& shared_mtx)
        : shared_mtx_{shared_mtx}
        , locked_{false}
    {
        lock();
    }

    ~ExclusiveLock()
    {
        unlock();
    }

    void lock()
    {
        if (!locked_)
        {
            std::unique_lock<std::mutex> lock{shared_mtx_.mtx_};
            shared_mtx_.cv_.wait(
                lock,
                [&]{ return (0 == shared_mtx_.shared_counter_) &&  !shared_mtx_.exclusively_locked_; });
            shared_mtx_.exclusively_locked_ = true;
            shared_mtx_.exclusive_thread_id_ = std::this_thread::get_id();
            locked_ = true;
        }
    }

    void unlock()
    {
        if (locked_)
        {
            std::unique_lock<std::mutex> lock{shared_mtx_.mtx_};
            shared_mtx_.exclusively_locked_ = false;
            shared_mtx_.cv_.notify_all();
            locked_ = false;
        }
    }

private:
    SharedMutex& shared_mtx_;
    bool locked_;
};


} // utils
} // uxr
} // eprosima

#endif // UXR_UTILS_SHAREDMUTEX_HPP_
