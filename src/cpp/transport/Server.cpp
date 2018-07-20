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

#include <micrortps/agent/transport/Server.hpp>
#include <micrortps/agent/processor/Processor.hpp>

namespace eprosima {
namespace micrortps {

Server::Server()
    : running_cond_(false),
      processor_(new Processor(this))
{}

Server::~Server()
{
    delete processor_;
}

bool Server::run()
{
    if (!init())
    {
        return false;
    }

    /* Scheduler initialization. */
    input_scheduler_.init();
    output_scheduler_.init();

    /* Thread initialization. */
    running_cond_ = true;
    receiver_thread_.reset(new std::thread(std::bind(&Server::receiver_loop, this)));
    sender_thread_.reset(new std::thread(std::bind(&Server::sender_loop, this)));
    processing_thread_.reset(new std::thread(std::bind(&Server::processing_loop, this)));
    heartbeat_thread_.reset(new std::thread(std::bind(&Server::heartbeat_loop, this)));

    return true;
}

void Server::stop()
{
    running_cond_ = false;
    input_scheduler_.deinit();
    output_scheduler_.deinit();
    if (receiver_thread_ && receiver_thread_->joinable())
    {
        receiver_thread_->join();
    }
    if (sender_thread_ && sender_thread_->joinable())
    {
        sender_thread_->join();
    }
    if (processing_thread_ && processing_thread_->joinable())
    {
        processing_thread_->join();
    }
    if (heartbeat_thread_ && heartbeat_thread_->joinable())
    {
        heartbeat_thread_->join();
    }
}

void Server::push_output_packet(OutputPacket output_packet)
{
    output_scheduler_.push(std::move(output_packet), 0);
}

void Server::receiver_loop()
{
    InputPacket input_packet;
    while (running_cond_)
    {
        if (recv_message(input_packet, -1))
        {
            input_scheduler_.push(std::move(input_packet), 0);
        }
    }
}

void Server::sender_loop()
{
    OutputPacket output_packet;
    while (running_cond_)
    {
        if (output_scheduler_.pop(output_packet))
        {
            send_message(output_packet);
        }
    }
}

void Server::processing_loop()
{
    InputPacket input_packet;
    while (running_cond_)
    {
        if (input_scheduler_.pop(input_packet))
        {
            processor_->process_input_packet(std::move(input_packet));
        }
    }
}

void Server::heartbeat_loop()
{
    while (running_cond_)
    {
        processor_->check_heartbeats();
        std::this_thread::sleep_for(std::chrono::milliseconds(MICRORTPS_HEARTBEAT_PERIOD));
    }
}

} // namespace micrortps
} // namespace eprosima
