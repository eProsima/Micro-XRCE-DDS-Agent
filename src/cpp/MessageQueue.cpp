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

#include "MessageQueue.h"

using eprosima::micrortps::MessageQueue;
using eprosima::micrortps::Message;

Message::Message(char* buf, size_t len)
{
    if (len <= buffer_.size())
    {
        std::copy(buf, buf + len, &buffer_[0]);
    }
}

MessageQueue::MessageQueue() : aborted_(false)
{
}

void MessageQueue::abort()
{
    std::lock_guard<std::mutex> queuelock(data_mutex_);
    aborted_ = true;
    condition_.notify_one();
}

Message MessageQueue::pop()
{
    std::unique_lock<std::mutex> queuelock(data_mutex_);
    condition_.wait(queuelock, [this]{
        return (!internal_queue_.empty() || aborted_);
    });
    Message message{};
    if (!aborted_)
    {
        message = internal_queue_.front();
        internal_queue_.pop();
    }
    return message;
}

void MessageQueue::push(const Message& new_message)
  {
    std::lock_guard<std::mutex> queuelock(data_mutex_);
    internal_queue_.push(new_message);
    condition_.notify_one();
  }
