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

#ifndef UXR_AGENT_UTILS_ARGUMENTPARSER_CPP_
#define UXR_AGENT_UTILS_ARGUMENTPARSER_CPP_

#include <uxr/agent/config.hpp>
#include <uxr/agent/utils/ArgumentParser.hpp>

// TODO(jamoralp): move definitions of ArgumentParser.hpp into this file, to maintain code coherence.

bool eprosima::uxr::agent::parser::utils::usage(
        const char* executable_name,
        bool no_help)
{
    std::string executable_name_str(executable_name);
    size_t pos = executable_name_str.rfind('/');
    if (std::string::npos != pos)
    {
        executable_name_str = executable_name_str.substr(pos + 1);
    }
    std::stringstream ss;
    ss << "Usage: '" << executable_name_str << " <udp4|udp6|tcp4|tpc6";
#ifndef _WIN32
    ss << "|canfd|serial|multiserial|pseudoterminal";
#endif // _WIN32
    ss << "> <<args>>'" << std::endl;
    if (no_help)
    {
        ss << "For a more detailed description about all the available arguments, ";
        ss << "please execute the agent with '-h/--help' option." << std::endl;
    }
    std::cout << ss.str();
    return false;
}

eprosima::uxr::agent::TransportKind eprosima::uxr::agent::parser::utils::check_transport(
        const char* transport)
{
    const std::unordered_map<std::string, TransportKind> valid_transports = {
    {"udp4", eprosima::uxr::agent::TransportKind::UDP4},
    {"udp6", eprosima::uxr::agent::TransportKind::UDP6},
    {"tcp4", eprosima::uxr::agent::TransportKind::TCP4},
    {"tcp6", eprosima::uxr::agent::TransportKind::TCP6},
#ifndef _WIN32
#ifdef UAGENT_SOCKETCAN_PROFILE
    {"canfd", eprosima::uxr::agent::TransportKind::CAN},
#endif // UAGENT_SOCKETCAN_PROFILE
    {"serial", eprosima::uxr::agent::TransportKind::SERIAL},
    {"multiserial", eprosima::uxr::agent::TransportKind::MULTISERIAL},
    {"pseudoterminal", eprosima::uxr::agent::TransportKind::PSEUDOTERMINAL},
#endif // _WIN32
    {"-h", eprosima::uxr::agent::TransportKind::HELP},
    {"--help", eprosima::uxr::agent::TransportKind::HELP}
    };

    if (valid_transports.find(transport) != valid_transports.end())
    {
        return valid_transports.at(transport);
    }
    return eprosima::uxr::agent::TransportKind::INVALID;
}

eprosima::uxr::Middleware::Kind eprosima::uxr::agent::parser::utils::get_mw_kind(
        const std::string& kind)
{
#ifdef UAGENT_FAST_PROFILE
    if ("dds" == kind)
    {
        return eprosima::uxr::Middleware::Kind::FASTDDS;
    }
    if ("rtps" == kind)
    {
        return eprosima::uxr::Middleware::Kind::FASTRTPS;
    }
#endif
#ifdef UAGENT_CED_PROFILE
    if ("ced" == kind)
    {
        return eprosima::uxr::Middleware::Kind::CED;
    }
#endif
    return eprosima::uxr::Middleware::Kind::NONE;
}

#endif // UXR_AGENT_UTILS_ARGUMENTPARSER_CPP_
