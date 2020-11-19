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

#include <uxr/agent/AgentInstance.hpp>
#include <uxr/agent/middleware/utils/Callbacks.hpp>

namespace eprosima {
namespace uxr {

AgentInstance::AgentInstance()
#ifdef UAGENT_CLI_PROFILE
    : app_("eProsima Micro XRCE-DDS Agent")
    , udpv4_subcmd_(app_)
    , udpv6_subcmd_(app_)
    , tcpv4_subcmd_(app_)
    , tcpv6_subcmd_(app_)
#ifndef _WIN32
    , termios_subcmd_(app_)
    , pseudo_serial_subcmd_(app_)
#endif  // _WIN32
    , exit_subcmd_(app_)
    , callback_factory_(callback_factory_.getInstance())
#else
    : callback_factory_(callback_factory_.getInstance())
#endif  // UAGENT_CLI_PROFILE
{
#ifdef UAGENT_CLI_PROFILE
    app_.require_subcommand(1, 1);
    app_.get_formatter()->column_width(42);
#endif
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
#ifndef _WIN32
    sigemptyset(&signals_);
    if (sigaddset(&signals_, SIGINT) && sigaddset(&signals_, SIGTERM))
    {
        std :: cerr << "Wrong signalset!" << std::endl;
        return false;
    }
#endif  // _WIN32
#ifdef UAGENT_CLI_PROFILE
    // Parse CLI arguments.
    std::stringstream ss;
    for (int i = 1; i < argc; ++i)
    {
        ss << argv[i] << " ";
    }
    std::string cli_input(ss.str());
    while (true)
    {
        try
        {
            app_.parse(cli_input);
            break;
        }
        catch(const CLI::ParseError& e)
        {
            app_.exit(e);
            std::cin.clear();
            std::cout << std::endl;
            std::cout << "Enter command: ";
            std::getline(std::cin, cli_input);
        }
    }
#else
    // Use built-in argument parser
    if (2 > argc)
    {
        agent::parser::utils::usage();
        return false;
    }
    const char* chosen_transport(argv[1]);
    agent::TransportKind valid_transport = agent::parser::utils::check_transport(chosen_transport);

    switch (valid_transport)
    {
        case agent::TransportKind::UDP4:
        {
            agent_thread_ = std::move(agent::create_agent_thread<UDPv4Agent>(argc, argv, valid_transport,
#ifndef _WIN32
                &signals_
#endif  // _WIN32
                ));
            break;
        }
        case agent::TransportKind::UDP6:
        {
            agent_thread_ = std::move(agent::create_agent_thread<UDPv6Agent>(argc, argv, valid_transport,
#ifndef _WIN32
                &signals_
#endif  // _WIN32
                ));
            break;
        }
        case agent::TransportKind::TCP4:
        {
            agent_thread_ = std::move(agent::create_agent_thread<TCPv4Agent>(argc, argv, valid_transport,
#ifndef _WIN32
                &signals_
#endif  // _WIN32
                ));
            break;
        }
        case agent::TransportKind::TCP6:
        {
            agent_thread_ = std::move(agent::create_agent_thread<TCPv6Agent>(argc, argv, valid_transport,
#ifndef _WIN32
                &signals_
#endif  // _WIN32
                ));
            break;
        }
#ifndef _WIN32
        case agent::TransportKind::SERIAL:
        {
            agent_thread_ = std::move(agent::create_agent_thread<TermiosAgent>(argc, argv,
                valid_transport, &signals_));
            break;
        }
        case agent::TransportKind::PSEUDOTERMINAL:
        {
            agent_thread_ = std::move(agent::create_agent_thread<PseudoTerminalAgent>(argc, argv,
                valid_transport, &signals_));
            break;
        }
#endif  // _WIN32
        case agent::TransportKind::HELP:
        {
            // TODO(jamoralp): avoid creating this object just to show help
            agent::parser::ArgumentParser<UDPv4Agent>(argc, argv, valid_transport).show_help();
            return false;
            break;
        }
        case agent::TransportKind::INVALID:
        {
            std::cerr << "Error: chosen transport '" << chosen_transport << "' is invalid!" << std::endl;
            agent::parser::utils::usage();
            return false;
        }
    }
#endif  // UAGENT_CLI_PROFILE
    return true;
}

void AgentInstance::run()
{
#ifdef UAGENT_CLI_PROFILE
    // Wait until exit.
#ifndef _WIN32
    int n_signal = 0;
    sigwait(&signals_, &n_signal);
#else
    std::cin.clear();
    char exit_flag = 0;
    while ('q' != exit_flag)
    {
        std::cin >> exit_flag;
    }
#endif  // _WIN32
#else
    agent_thread_.join();
#endif  // UAGENT_CLI_PROFILE
}

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
template <> \
UXR_AGENT_EXPORT void AgentInstance::add_middleware_callback<__VA_ARGS__>( \
    const Middleware::Kind &, \
    const middleware::CallbackKind &, \
    std::function<void(__VA_ARGS__)> &&);

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

} // namespace uxr
} // namespace eprosima