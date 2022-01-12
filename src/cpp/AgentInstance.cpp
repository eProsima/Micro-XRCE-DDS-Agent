// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <uxr/agent/config.hpp>
#include <uxr/agent/AgentInstance.hpp>
#include <uxr/agent/middleware/utils/Callbacks.hpp>

namespace eprosima {
namespace uxr {

AgentInstance::AgentInstance()
    : callback_factory_(callback_factory_.getInstance())
{
}

AgentInstance& AgentInstance::getInstance()
{
    static AgentInstance instance;
    return instance;
}

bool AgentInstance::create(
        int argc,
        char** argv)
{
    if (2 > argc)
    {
        agent::parser::utils::usage(argv[0]);
        return false;
    }
    const char* chosen_transport(argv[1]);
    agent::TransportKind valid_transport = agent::parser::utils::check_transport(chosen_transport);

    switch (valid_transport)
    {
        case agent::TransportKind::UDP4:
        {
            agent_thread_ = std::move(agent::create_agent_thread<UDPv4Agent>(argc, argv, exit_signal, valid_transport));
            break;
        }
        case agent::TransportKind::UDP6:
        {
            agent_thread_ = std::move(agent::create_agent_thread<UDPv6Agent>(argc, argv, exit_signal, valid_transport));
            break;
        }
        case agent::TransportKind::TCP4:
        {
            agent_thread_ = std::move(agent::create_agent_thread<TCPv4Agent>(argc, argv, exit_signal, valid_transport));
            break;
        }
        case agent::TransportKind::TCP6:
        {
            agent_thread_ = std::move(agent::create_agent_thread<TCPv6Agent>(argc, argv, exit_signal, valid_transport));
            break;
        }
#ifndef _WIN32
#ifdef UAGENT_SOCKETCAN_PROFILE
        case agent::TransportKind::CAN:
        {
            agent_thread_ = std::move(agent::create_agent_thread<CanAgent>(argc, argv, exit_signal, valid_transport));
            break;
        }
#endif // UAGENT_SOCKETCAN_PROFILE
        case agent::TransportKind::SERIAL:
        {
            agent_thread_ = std::move(agent::create_agent_thread<TermiosAgent>(argc, argv, exit_signal, valid_transport));
            break;
        }
        case agent::TransportKind::MULTISERIAL:
        {
            agent_thread_ = std::move(agent::create_agent_thread<MultiTermiosAgent>(argc, argv, exit_signal, valid_transport));
            break;
        }
        case agent::TransportKind::PSEUDOTERMINAL:
        {
            agent_thread_ = std::move(agent::create_agent_thread<PseudoTerminalAgent>(argc, argv, exit_signal, valid_transport));
            break;
        }
#endif  // _WIN32
        case agent::TransportKind::HELP:
        {
            // TODO(jamoralp): avoid creating this object just to show help
            agent::parser::ArgumentParser<UDPv4Agent>(argc, argv, valid_transport).show_help();
            return false;
        }
        case agent::TransportKind::INVALID:
        {
            std::cerr << "Error: chosen transport '" << chosen_transport << "' is invalid!" << std::endl;
            agent::parser::utils::usage(argv[0]);
            return false;
        }
    }

    return true;
}

void AgentInstance::run()
{
    agent_thread_.join();
}

#ifndef _WIN32
void AgentInstance::stop()
{
    if (agent_thread_.joinable())
    {
        exit_signal.notify_all();
        agent_thread_.join();
    }    
}
#endif

template <typename ... Args>
void AgentInstance::add_middleware_callback(
        const Middleware::Kind& middleware_kind,
        const middleware::CallbackKind& callback_kind,
        std::function<void (Args ...)>&& callback_function)
{
    callback_factory_.add_callback(middleware_kind, callback_kind, std::move(callback_function));
}

// Specific template specializations for used callback signatures.
#define AGENTINSTANCE_ADD_MW_CB(...) \
template \
UXR_AGENT_EXPORT void AgentInstance::add_middleware_callback<__VA_ARGS__>( \
    const Middleware::Kind &, \
    const middleware::CallbackKind &, \
    std::function<void(__VA_ARGS__)> &&);

#ifdef UAGENT_FAST_PROFILE
AGENTINSTANCE_ADD_MW_CB(
    const eprosima::fastrtps::Participant *)

AGENTINSTANCE_ADD_MW_CB(
    const eprosima::fastrtps::Participant *,
    const eprosima::fastrtps::Publisher *)

AGENTINSTANCE_ADD_MW_CB(
    const eprosima::fastrtps::Participant *,
    const eprosima::fastrtps::Subscriber *)

AGENTINSTANCE_ADD_MW_CB(
    const eprosima::fastrtps::Participant *,
    const eprosima::fastrtps::Publisher *,
    const eprosima::fastrtps::Subscriber *)

AGENTINSTANCE_ADD_MW_CB(
    const eprosima::fastdds::dds::DomainParticipant *)

AGENTINSTANCE_ADD_MW_CB(
    const eprosima::fastdds::dds::DomainParticipant *,
    const eprosima::fastdds::dds::DataWriter *)

AGENTINSTANCE_ADD_MW_CB(
    const eprosima::fastdds::dds::DomainParticipant *,
    const eprosima::fastdds::dds::DataReader *)

AGENTINSTANCE_ADD_MW_CB(
    const eprosima::fastdds::dds::DomainParticipant *,
    const eprosima::fastdds::dds::DataWriter *,
    const eprosima::fastdds::dds::DataReader *)
#endif  // UAGENT_FAST_PROFILE

} // namespace uxr
} // namespace eprosima