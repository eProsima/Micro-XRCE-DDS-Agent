#include "MessageQueue.h"

using namespace eprosima::micrortps;

MessageQueue::MessageQueue() : aborted_(false)
{
}

void MessageQueue::abort()
{
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
    if (!aborted_ && !internal_queue_.empty())
    {
        message = internal_queue_.front();
        internal_queue_.pop();
    }
    return message;
}

void MessageQueue::push(const Message& new_message)
  {
    std::unique_lock<std::mutex> queuelock(data_mutex_);
    internal_queue_.push(new_message);
    queuelock.unlock();
    condition_.notify_one();
  }
  