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

#include <uxr/agent/transport/Server.hpp>
#include <uxr/agent/config.hpp>
#include <uxr/agent/processor/Processor.hpp>
#include <uxr/agent/Root.hpp>
#include <uxr/agent/logger/Logger.hpp>

#include <uxr/agent/transport/endpoint/IPv4EndPoint.hpp>
#include <uxr/agent/transport/endpoint/IPv6EndPoint.hpp>
#include <uxr/agent/transport/endpoint/CanEndPoint.hpp>
#include <uxr/agent/transport/endpoint/SerialEndPoint.hpp>
#include <uxr/agent/transport/endpoint/MultiSerialEndPoint.hpp>
#include <uxr/agent/transport/endpoint/CustomEndPoint.hpp>

#include <functional>

#define RECEIVE_TIMEOUT 1000   // Milliseconds

namespace eprosima {
namespace uxr {

extern template class Processor<IPv4EndPoint>;
extern template class Processor<IPv6EndPoint>;
extern template class Processor<CanEndPoint>;
extern template class Processor<SerialEndPoint>;
extern template class Processor<MultiSerialEndPoint>;
extern template class Processor<CustomEndPoint>;

template<typename EndPoint>
Server<EndPoint>::Server(Middleware::Kind middleware_kind)
    : processor_(new Processor<EndPoint>(*this, *root_, middleware_kind))
    , running_cond_(false)
    , input_scheduler_(SERVER_QUEUE_MAX_SIZE)
    , output_scheduler_(SERVER_QUEUE_MAX_SIZE)
    , transport_rc_{TransportRc::ok}
    , error_mtx_{}
    , error_cv_{}
{}

template<typename EndPoint>
Server<EndPoint>::~Server()
{
    delete processor_;
}

template<typename EndPoint>
bool Server<EndPoint>::start()
{
    std::lock_guard<std::mutex> lock(mtx_);

    /* Init server. */
    if (!init())
    {
        return false;
    }

    /* Scheduler initialization. */
    input_scheduler_.init();
    input_scheduler_.set_priority_size(1, 1); // Priority 1 used for heartbeats
    output_scheduler_.init();

    /* Thread initialization. */
    running_cond_ = true;
    error_handler_thread_ = std::thread(&Server::error_handler_loop, this);
    receiver_thread_ = std::thread(&Server::receiver_loop, this);
    sender_thread_ = std::thread(&Server::sender_loop, this);
    processing_thread_ = std::thread(&Server::processing_loop, this);
    heartbeat_thread_ = std::thread(&Server::heartbeat_loop, this);

    return true;
}

template<typename EndPoint>
bool Server<EndPoint>::stop()
{
    std::lock_guard<std::mutex> lock(mtx_);
    running_cond_ = false;

    /* Stop input and output queues. */
    input_scheduler_.deinit();
    output_scheduler_.deinit();

    error_cv_.notify_all();

    /* Join threads. */
    if (receiver_thread_.joinable())
    {
        receiver_thread_.join();
    }
    if (sender_thread_.joinable())
    {
        sender_thread_.join();
    }
    if (processing_thread_.joinable())
    {
        processing_thread_.join();
    }
    if (heartbeat_thread_.joinable())
    {
        heartbeat_thread_.join();
    }
    if (error_handler_thread_.joinable())
    {
        error_handler_thread_.join();
    }

    /* Close servers. */
    bool rv = true;
#ifdef UAGENT_DISCOVERY_PROFILE
    if (has_discovery())
    {
        rv = fini_discovery() && rv;
    }
#endif
#ifdef UAGENT_P2P_PROFILE
    if (has_p2p())
    {
        rv = fini_p2p() && rv;
    }
#endif
    rv = fini() && rv;
    return rv;
}

#ifdef UAGENT_DISCOVERY_PROFILE
template<typename EndPoint>
bool Server<EndPoint>::enable_discovery(uint16_t discovery_port)
{
    bool rv = false;
    if (running_cond_)
    {
        rv = init_discovery(discovery_port);
    }
    return rv;
}

template<typename EndPoint>
bool Server<EndPoint>::disable_discovery()
{
    return fini_discovery();
}
#endif

#ifdef UAGENT_P2P_PROFILE
template<typename EndPoint>
bool Server<EndPoint>::enable_p2p(uint16_t p2p_port)
{
    bool rv = false;
    if (running_cond_)
    {
        rv = init_p2p(p2p_port);
    }
    return rv;
}

template<typename EndPoint>
bool Server<EndPoint>::disable_p2p()
{
    return fini_p2p();
}
#endif

template<typename EndPoint>
void Server<EndPoint>::push_output_packet(
        OutputPacket<EndPoint>&& output_packet)
{
    if (output_packet.message)
    {
        output_scheduler_.push(std::move(output_packet), 0);
    }
}

template<typename EndPoint>
void Server<EndPoint>::receiver_loop()
{
    InputPacket<EndPoint> input_packet{};
    while (running_cond_)
    {
        TransportRc transport_rc = TransportRc::ok;
        if (recv_message(input_packet, RECEIVE_TIMEOUT, transport_rc))
        {
            if(input_packet.message->is_valid_xrce_message() && 1U == input_packet.message->count_submessages() && dds::xrce::HEARTBEAT == input_packet.message->get_submessage_id()){
                input_scheduler_.push(std::move(input_packet), 1);
            }
            else
            {
                input_scheduler_.push(std::move(input_packet), 0);
            }
        }
        else if(running_cond_)
        {
            if (TransportRc::server_error == transport_rc)
            {
                std::unique_lock<std::mutex> lock(error_mtx_);
                transport_rc_ = transport_rc;
                error_cv_.notify_one();
                error_cv_.wait(lock);
            }
        }
    }
}

template<>
void Server<MultiSerialEndPoint>::receiver_loop()
{
    std::vector<InputPacket<MultiSerialEndPoint>> input_packet;

    while (running_cond_)
    {
        TransportRc transport_rc = TransportRc::ok;
        if (recv_message(input_packet, RECEIVE_TIMEOUT, transport_rc))
        {
            for (auto & element : input_packet)
            {
                input_scheduler_.push(std::move(element), 0);
            }
        }
        else if(running_cond_)
        {
            if (TransportRc::server_error == transport_rc)
            {
                std::unique_lock<std::mutex> lock(error_mtx_);
                transport_rc_ = transport_rc;
                error_cv_.notify_one();
                error_cv_.wait(lock);
            }
        }

        input_packet.clear();
    }
}

template<typename EndPoint>
void Server<EndPoint>::sender_loop()
{
    OutputPacket<EndPoint> output_packet{};
    while (running_cond_)
    {
        if (output_scheduler_.pop(output_packet))
        {
            TransportRc transport_rc = TransportRc::ok;
            if (!send_message(output_packet, transport_rc))
            {
                if (TransportRc::server_error == transport_rc && running_cond_)
                {
                    std::unique_lock<std::mutex> lock(error_mtx_);
                    transport_rc_ = transport_rc;
                    output_scheduler_.push_front(std::move(output_packet), 0);
                    error_cv_.notify_one();
                    error_cv_.wait(lock);
                }
            }
        }
    }
}

template<typename EndPoint>
void Server<EndPoint>::processing_loop()
{
    InputPacket<EndPoint> input_packet;
    while (running_cond_)
    {
        if (input_scheduler_.pop(input_packet))
        {
            processor_->process_input_packet(std::move(input_packet));
        }
    }
}

template<typename EndPoint>
void Server<EndPoint>::heartbeat_loop()
{
    while (running_cond_)
    {
        processor_->check_heartbeats();
        std::this_thread::sleep_for(std::chrono::milliseconds(HEARTBEAT_PERIOD));
    }
}

template<typename EndPoint>
void Server<EndPoint>::error_handler_loop()
{
    while (running_cond_)
    {
        std::unique_lock<std::mutex> lock(error_mtx_);
        error_cv_.wait(lock, [&](){ return !running_cond_ || (transport_rc_ == TransportRc::server_error); });
        if (running_cond_)
        {
            bool error_handled = handle_error(transport_rc_);
            while (running_cond_ && !error_handled)
            {
                error_handled = handle_error(transport_rc_);
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
            transport_rc_ = TransportRc::ok;
            error_cv_.notify_all();
        }
    }
}

template class Server<IPv4EndPoint>;
template class Server<IPv6EndPoint>;
template class Server<CanEndPoint>;
template class Server<SerialEndPoint>;
template class Server<MultiSerialEndPoint>;
template class Server<CustomEndPoint>;

} // namespace uxr
} // namespace eprosima
