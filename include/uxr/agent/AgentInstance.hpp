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

#ifndef UXR_AGENT_AGENT_INSTANCE_HPP_
#define UXR_AGENT_AGENT_INSTANCE_HPP_

#include <uxr/agent/config.hpp>

#ifdef UAGENT_CLI_PROFILE
#include <uxr/agent/utils/CLI.hpp>
#else
#include <uxr/agent/utils/ArgumentParser.hpp>
#endif  // UAGENT_CLI_PROFILE

#include <csignal>

namespace eprosima {
namespace uxr {

/**
 * @brief   Singleton class to manage the launch process of a MicroXRCE-DDS Agent.
 */
class AgentInstance
{
public:
    /**
     * @brief   Default constructor.
     */
    UXR_AGENT_EXPORT AgentInstance();
    /**
     * @brief   AgentInstance class shall not be copy constructible.
     */
    UXR_AGENT_EXPORT AgentInstance(
            const AgentInstance &) = delete;

    UXR_AGENT_EXPORT AgentInstance(
            AgentInstance &&) = delete;

    /**
     * @brief   AgentInstance class shall not be copy assignable.
     */
    UXR_AGENT_EXPORT AgentInstance& operator =(
            const AgentInstance &) = delete;

    UXR_AGENT_EXPORT AgentInstance& operator =(
            AgentInstance &&) = delete;

    /**
     * @brief   Get instance associated to this class.
     * @return  Static reference to the singleton AgentInstance object.
     */
    UXR_AGENT_EXPORT static AgentInstance& getInstance();

    /**
     * @brief       Create an Agent instance, based on provided input parameters from user.
     * @param[in]   argc Number of available parameters introduced by the user.
     * @param[in]   argv List of parameters to be parsed to instantiate an Agent.
     * @return      Boolean value indicating if a Micro XRCE-DDS Agent was instantiated successfully.
     */
    UXR_AGENT_EXPORT bool create(
            int argc,
            char** argv);

    /**
     * @brief   Run the created agent until finished via user interrupt or process error.
     */
    UXR_AGENT_EXPORT void run();

private:
#ifdef UAGENT_CLI_PROFILE
    CLI::App app_;
    cli::UDPv4Subcommand udpv4_subcmd_;
    cli::UDPv6Subcommand udpv6_subcmd_;
    cli::TCPv4Subcommand tcpv4_subcmd_;
    cli::TCPv6Subcommand tcpv6_subcmd_;
#ifndef _WIN32
    cli::TermiosSubcommand termios_subcmd_;
    cli::PseudoTerminalSubcommand pseudo_serial_subcmd_;
#endif  // _WIN32
    cli::ExitSubcommand exit_subcmd_;
#else
    std::thread agent_thread_;
#endif  // UAGENT_CLI_PROFILE
#ifndef _WIN32
    sigset_t signals_;
#endif  // _WIN32
};
} // uxr
} // eprosima

#endif  // UXR_AGENT_AGENT_INSTANCE_HPP_