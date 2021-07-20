// Copyright 2021-present Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <uxr/agent/transport/custom/CustomAgent.hpp>

#include <functional>

namespace eprosima {
namespace uxr {

namespace {
const std::string transport_rc_to_str(
        const TransportRc& transport_rc)
{
    switch (transport_rc)
    {
        case TransportRc::connection_error:
        {
            return std::string("connection error");
        }
        case TransportRc::timeout_error:
        {
            return std::string("timeout error");
        }
        case TransportRc::server_error:
        {
            return std::string("server error");
        }
        default:
        {
            return std::string();
        }
    }
}

} // anonymous namespace

CustomAgent::CustomAgent(
        const std::string& name,
        CustomEndPoint* endpoint,
        Middleware::Kind middleware_kind,
        bool framing,
        InitFunction& init_function,
        FiniFunction& fini_function,
        SendMsgFunction& send_msg_function,
        RecvMsgFunction& recv_msg_function)
    : Server<CustomEndPoint>(middleware_kind)
    , name_(name)
    , recv_endpoint_(endpoint)
    , send_endpoint_(nullptr)
    , custom_init_func_(init_function)
    , custom_fini_func_(fini_function)
    , custom_send_msg_func_(send_msg_function)
    , custom_recv_msg_func_(recv_msg_function)
    , framing_(framing)
    , framing_io_(0x00,
        [&](
                uint8_t* buffer,
                size_t message_length,
                TransportRc& transport_rc) -> ssize_t
        {
            return custom_send_msg_func_(
                send_endpoint_,
                buffer,
                message_length,
                transport_rc);
        },
        [&](
                uint8_t* buffer,
                size_t buffer_length,
                int timeout,
                TransportRc& transport_rc) -> ssize_t
        {
            return custom_recv_msg_func_(
                recv_endpoint_,
                buffer,
                buffer_length,
                timeout,
                transport_rc);
        })
{
}

CustomAgent::~CustomAgent()
{
    try
    {
        this->stop();
    }
    catch (const std::exception& e)
    {
        UXR_AGENT_LOG_CRITICAL(
            UXR_DECORATE_RED("Error stopping custom agent server"),
            "{} agent exception: {}",
            name_, e.what());
    }
}

bool CustomAgent::init()
{
    try
    {
        bool user_init_res = custom_init_func_();

        if (user_init_res)
        {
            UXR_AGENT_LOG_INFO(
                UXR_DECORATE_GREEN("Custom agent status: opened"),
                "{} agent running",
                name_);
        }
        else
        {
            UXR_AGENT_LOG_ERROR(
                UXR_DECORATE_RED("Error initializing custom agent server"),
                "{} agent error",
                name_);
        }

        return user_init_res;
    }
    catch (const std::exception& e)
    {
        UXR_AGENT_LOG_CRITICAL(
            UXR_DECORATE_RED("Error initializing custom agent server"),
            "{} agent exception: {}",
            name_, e.what());

        return false;
    }
}

bool CustomAgent::fini()
{
    try
    {
        bool user_fini_res = custom_fini_func_();

        if (user_fini_res)
        {
            UXR_AGENT_LOG_INFO(
                UXR_DECORATE_GREEN("Custom agent status: closed"),
                "{} agent closed",
                name_);
        }
        else
        {
            UXR_AGENT_LOG_ERROR(
                UXR_DECORATE_RED("Error terminating custom agent server"),
                "{} agent error",
                name_);
        }

        return user_fini_res;
    }
    catch (const std::exception& e)
    {
        UXR_AGENT_LOG_CRITICAL(
            UXR_DECORATE_RED("Error terminating custom agent server"),
            "{} agent exception: {}",
            name_, e.what());

        return false;
    }
}

bool CustomAgent::recv_message(
        InputPacket<CustomEndPoint>& input_packet,
        int timeout,
        TransportRc& transport_rc)
{
    // Reset recv_endpoint_ members before receiving a new message.
    recv_endpoint_->reset();

    try
    {
        ssize_t recv_bytes = 0;
        if (framing_)
        {
            uint8_t remote_addr = 0x00;

            do
            {
                recv_bytes = framing_io_.read_framed_msg(
                    buffer_, SERVER_BUFFER_SIZE, remote_addr, timeout, transport_rc);
            }
            while ((0 == recv_bytes) && (0 < timeout));
        }
        else
        {
            recv_bytes = custom_recv_msg_func_(
                recv_endpoint_, buffer_, SERVER_BUFFER_SIZE, timeout, transport_rc);
        }

        bool success = (0 < recv_bytes && TransportRc::ok == transport_rc);
        if (success)
        {
            // User must have filled all the members of the endpoint.
            recv_endpoint_->check_non_empty_members();

            input_packet.message.reset(
                new eprosima::uxr::InputMessage(
                    buffer_, static_cast<size_t>(recv_bytes)));
            input_packet.source = *recv_endpoint_;

            uint32_t raw_client_key = 0u;
            this->get_client_key(input_packet.source, raw_client_key);

            std::stringstream ss;
            ss << UXR_COLOR_YELLOW << "[==>> " << name_ << " <<==]" << UXR_COLOR_RESET;
            UXR_AGENT_LOG_MESSAGE(
                ss.str(),
                raw_client_key,
                input_packet.message->get_buf(),
                input_packet.message->get_len());
        }
        else if (TransportRc::timeout_error != transport_rc)
        {
            // Printing a trace for timeout_error would fill the log with too much messages.
            std::stringstream ss;
            ss << UXR_COLOR_RED << "Error while receiving message: "
               << transport_rc_to_str(transport_rc) << UXR_COLOR_RESET;
            UXR_AGENT_LOG_ERROR(
                ss.str(),
                "{} agent error",
                name_);
        }

        return success;
    }
    catch (const std::exception& e)
    {
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("Error while receiving message"),
            "custom {} agent, exception: {}",
            name_, e.what());
        transport_rc = TransportRc::server_error;

        return false;
    }
}

bool CustomAgent::send_message(
        OutputPacket<CustomEndPoint> output_packet,
        TransportRc& transport_rc)
{
    try
    {
        ssize_t sent_bytes;
        if (framing_)
        {
            send_endpoint_ = &output_packet.destination;
            sent_bytes = framing_io_.write_framed_msg(
                output_packet.message->get_buf(),
                output_packet.message->get_len(),
                0x00,
                transport_rc);
            send_endpoint_ = nullptr;
        }
        else
        {
            sent_bytes = custom_send_msg_func_(
                &output_packet.destination,
                output_packet.message->get_buf(),
                output_packet.message->get_len(),
                transport_rc);
        }

        bool success = (output_packet.message->get_len() == static_cast<size_t>(sent_bytes));
        if (success)
        {
            uint32_t raw_client_key = 0u;
            this->get_client_key(output_packet.destination, raw_client_key);

            std::stringstream ss;
            ss << UXR_COLOR_YELLOW << "[** <<" << name_ << ">> **]" << UXR_COLOR_RESET;
            UXR_AGENT_LOG_MESSAGE(
                ss.str(),
                raw_client_key,
                output_packet.message->get_buf(),
                output_packet.message->get_len());
        }
        else
        {
            std::stringstream ss;
            ss << UXR_COLOR_RED
               << "Error while sending message: "
               << transport_rc_to_str(transport_rc)
               << ". Expected to send "
               << output_packet.message->get_len()
               << " bytes, but sent "
               << sent_bytes
               << "instead"
               << UXR_COLOR_RESET;
            UXR_AGENT_LOG_ERROR(
                ss.str(),
                "{} agent error",
                name_);
        }

        return success;
    }
    catch (const std::exception& e)
    {
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("Error while sending message"),
            "custom {} agent, exception: {}",
            name_, e.what());

        return false;
    }
}

bool CustomAgent::handle_error(
        TransportRc transport_rc)
{
    std::stringstream ss;
    ss << UXR_COLOR_RED << "Recovering from error: "
       << transport_rc_to_str(transport_rc) << UXR_COLOR_RESET;
    UXR_AGENT_LOG_ERROR(
        ss.str(),
        "{} agent error",
        name_);
    return fini() && init();
}

} // namespace uxr
} // namespace eprosima