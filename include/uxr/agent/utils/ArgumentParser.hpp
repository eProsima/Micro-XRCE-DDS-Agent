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

#ifndef UXR_AGENT_UTILS_ARGUMENTPARSER_HPP_
#define UXR_AGENT_UTILS_ARGUMENTPARSER_HPP_

#include <sstream>
#include <fstream>
#include <csignal>
#include <type_traits>
#include <unordered_map>
#include <uxr/agent/transport/Server.hpp>

#include  <uxr/agent/utils/AgentServer.hpp>

#include <fcntl.h>
#include <sys/stat.h>

#define DEFAULT_MIDDLEWARE      "dds"
#define DEFAULT_VERBOSE_LEVEL   4
#define DEFAULT_DISCOVERY_PORT  7400
#define DEFAULT_BAUDRATE_LEVEL  "115200"

namespace eprosima {
namespace uxr {
namespace agent {

template <typename AgentKind>
std::thread create_agent_thread(
        int argc,
        char** argv,
#ifndef _WIN32
        TransportKind transport_kind,
        const sigset_t* signals);
#else
        TransportKind transport_kind);
#endif // _WIN32

namespace parser {
namespace utils {

bool usage(
        const char* executable_name,
        bool no_help = true);

TransportKind check_transport(
        const char* transport);

Middleware::Kind get_mw_kind(
        const std::string& kind);
} // namespace utils

using dummy_type = uint8_t;

enum class ArgumentKind
{
    NO_VALUE,
    VALUE,
};

enum class ParseResult
{
    INVALID,
    NOT_FOUND,
    VALID,
    HELP,
};

/*************************************************************************************************
 * Generic command line argument representation
 *************************************************************************************************/
// TODO(jamoralp): add proper documentation for this whole file.
template <typename T>
class Argument
{
public:
    Argument(
            const char* short_alias,
            const char* long_alias,
            ArgumentKind kind = ArgumentKind::VALUE,
            const std::set<T> allowed_values = {},
            bool enabled_by_default = true)
        : argument_kind_(kind)
        , short_alias_(short_alias)
        , long_alias_(long_alias)
        , value_()
        , has_default_value_(false)
        , enabled_by_default_(enabled_by_default)
        , parse_found_(false)
        , allowed_values_(allowed_values)
    {
    }

    Argument(
            const char* short_alias,
            const char* long_alias,
            const T& default_value,
            const std::set<T> allowed_values = {},
            bool enabled_by_default = true)
        : argument_kind_(ArgumentKind::VALUE)
        , short_alias_(short_alias)
        , long_alias_(long_alias)
        , value_(default_value)
        , has_default_value_(true)
        , enabled_by_default_(enabled_by_default)
        , parse_found_(false)
        , allowed_values_(allowed_values)
    {
    }

    ~Argument() = default;

    const T& value() const
    {
        return value_;
    }

    T& value()
    {
        return value_;
    }

    bool found()
    {
        return parse_found_;
    }

    ParseResult parse_argument(
            int argc,
            char** argv)
    {
        for (int position = 0; position < argc; ++position)
        {
            /*
             * We are distinguishing two cases: short opt and long opt cases. This is done because,
             * for short opt option, value can be specified directy attached, with no whitespace between
             * the option tag and its associated value.
             */
            const std::string opt(argv[position]);
            std::string value_str("");
            bool exists_value(false);

            if (short_alias_ == opt || long_alias_ == opt)
            {
                if (ArgumentKind::VALUE != argument_kind_)
                {
                    parse_found_ = true;
                    return ParseResult::VALID;
                }

                if (argc > (++position))
                {
                    exists_value = true;
                    value_str = argv[position];

                    /**
                     * It might occur that an argument is specified to enable some behaviour but we want
                     * to use the default value. Handle that case.
                     */
                    if (std::string::npos != value_str.find("-"))
                    {
                        if (has_default_value_)
                        {
                            parse_found_ = true;
                            return ParseResult::VALID;
                        }
                        else
                        {
                            parse_found_ = false;
                            return ParseResult::INVALID;
                        }
                    }
                }
                else if (has_default_value_)
                {
                    parse_found_ = true;
                    return ParseResult::VALID;
                }
            }
            else if (0 == opt.find(short_alias_))
            {
                exists_value = true;
                if (ArgumentKind::VALUE != argument_kind_)
                {
                    parse_found_ = false;
                    return ParseResult::INVALID;
                }

                value_str = opt.substr(short_alias_.length());
            }

            if (exists_value && std::is_integral<T>::value)
            {
                if (std::all_of(value_str.begin(), value_str.end(), ::isdigit))
                {
                    value_ = static_cast<T>(stol(value_str));
                    if (!allowed_values_.empty() && allowed_values_.find(value_) == allowed_values_.end())
                    {
                        std::stringstream ss;
                        ss << "Warning: introduced value for argument '" << long_alias_;
                        ss << "' is not allowed. Please choose between {";
                        for (const auto& allowed_value_ : allowed_values_)
                        {
                            ss << allowed_value_ << ((*allowed_values_.rbegin() != allowed_value_) ? ", " : "}.");
                        }
                        ss << std::endl;
                        std::cerr << ss.str();
                        return ParseResult::INVALID;
                    }
                    parse_found_ = true;
                    return ParseResult::VALID;
                }
                else
                {
                    return ParseResult::INVALID;
                }
            }
        }

        /**
         * Argument not found, but maybe it is enabled by default.
         */
        if (enabled_by_default_ && has_default_value_)
        {
            parse_found_ = true;
            return ParseResult::VALID;
        }

        return ParseResult::NOT_FOUND;
    }

    const std::string get_help() const
    {
        std::stringstream ss;
        ss << short_alias_ << "/" << long_alias_;
        if (ArgumentKind::VALUE == argument_kind_)
        {
            ss << " <value>";
        }
        if (!allowed_values_.empty())
        {
            ss << " (" << *allowed_values_.begin() << " - " << *allowed_values_.rbegin() << ")";
        }
        if (has_default_value_)
        {
            ss << " [default: '" << value_ << "']";
        }
        ss << ".";
        return ss.str();
    }

private:
    ArgumentKind argument_kind_;
    std::string short_alias_;
    std::string long_alias_;
    T value_;
    bool has_default_value_;
    bool enabled_by_default_;
    bool parse_found_;
    std::set<T> allowed_values_;
};

/*************************************************************************************************
 * Specialized command line argument representation for std::string
 *************************************************************************************************/
template <>
class Argument<std::string>
{
public:
    Argument(
            const char* short_alias,
            const char* long_alias,
            const std::set<std::string> allowed_values = {},
            ArgumentKind kind = ArgumentKind::VALUE,
            bool enabled_by_default = true)
        : argument_kind_(kind)
        , short_alias_(short_alias)
        , long_alias_(long_alias)
        , value_()
        , has_default_value_(false)
        , enabled_by_default_(enabled_by_default)
        , parse_found_(false)
        , allowed_values_(allowed_values)
    {
    }

    Argument(
            const char* short_alias,
            const char* long_alias,
            const std::string& default_value,
            const std::set<std::string> allowed_values = {},
            bool enabled_by_default = true)
        : argument_kind_(ArgumentKind::VALUE)
        , short_alias_(short_alias)
        , long_alias_(long_alias)
        , value_(default_value)
        , has_default_value_(true)
        , enabled_by_default_(enabled_by_default)
        , parse_found_(false)
        , allowed_values_(allowed_values)
    {
    }

    ~Argument() = default;

    const std::string& value() const
    {
        return value_;
    }

    std::string& value()
    {
        return value_;
    }

    bool found()
    {
        return parse_found_;
    }

ParseResult parse_argument(
            int argc,
            char** argv)
    {
        for (int position = 0; position < argc; ++position)
        {
            if (0 == strcmp(argv[position], short_alias_.c_str()) ||
                0 == strcmp(argv[position], long_alias_.c_str()))
            {
                if (ArgumentKind::VALUE == argument_kind_)
                {
                    if (argc > (++position))
                    {
                        value_ = std::string(argv[position]);
                        if (!allowed_values_.empty() &&
                            allowed_values_.find(value_) == allowed_values_.end())
                        {
                            std::stringstream ss;
                            ss << "Warning: introduced value for argument '" << long_alias_;
                            ss << "' is not allowed. Please choose between {";
                            for (const auto& allowed_value_ : allowed_values_)
                            {
                                ss << allowed_value_ << ((*allowed_values_.rbegin() != allowed_value_) ? ", " : "}.");
                            }
                            ss << std::endl;
                            std::cerr << ss.str();
                            return ParseResult::INVALID;
                        }
                        parse_found_ = true;
                        return ParseResult::VALID;
                    }
                    else
                    {
                        return ParseResult::INVALID;
                    }
                }
                else
                {
                    parse_found_ = true;
                    return ParseResult::VALID;
                }
            }
        }

        /**
         * Argument not found, but maybe it is enabled by default.
         */
        if (enabled_by_default_ && has_default_value_)
        {
            parse_found_ = true;
            return ParseResult::VALID;
        }

        return ParseResult::NOT_FOUND;
    }

    const std::string get_help() const
    {
        std::stringstream ss;
        ss << short_alias_ << "/" << long_alias_;
        if (ArgumentKind::VALUE == argument_kind_)
        {
            ss << " <value>";
        }
        if (!allowed_values_.empty())
        {
            ss << " (";
            for (const auto& allowed_value_ : allowed_values_)
            {
                ss << allowed_value_ << ((*allowed_values_.rbegin() != allowed_value_) ? ", " : ")");
            }
        }
        if (has_default_value_)
        {
            ss << " [default: '" << value_ << "']";
        }
        ss << ".";
        return ss.str();
    }

private:
    ArgumentKind argument_kind_;
    std::string short_alias_;
    std::string long_alias_;
    std::string value_;
    bool has_default_value_;
    bool enabled_by_default_;
    bool parse_found_;
    std::set<std::string> allowed_values_;
};

/*************************************************************************************************
 * Common arguments for each available transport
 *************************************************************************************************/
template <typename AgentType>
class CommonArgs
{
public:
    CommonArgs()
        : help_("-h", "--help", ArgumentKind::NO_VALUE)
        , middleware_("-m", "--middleware", std::string(DEFAULT_MIDDLEWARE),
            {"dds", "ced", "rtps"})
        , refs_("-r", "--refs")
        , verbose_("-v", "--verbose", static_cast<uint16_t>(DEFAULT_VERBOSE_LEVEL),
            {0, 1, 2, 3, 4, 5, 6})
#ifdef UAGENT_DISCOVERY_PROFILE
        , discovery_("-d", "--discovery", static_cast<uint16_t>(DEFAULT_DISCOVERY_PORT), {}, false)
#endif
#ifdef UAGENT_P2P_PROFILE
        , p2p_("-P", "--p2p")
#endif
    {
    }

    const std::string middleware() const
    {
        return middleware_.value();
    }

    std::pair<bool, bool> parse(
            int argc,
            char** argv)
    {
        std::pair<bool, bool> result(true, false);
        ParseResult parse_help = help_.parse_argument(argc, argv);
        if (ParseResult::VALID == parse_help)
        {
            result.first = true;
            result.second = true;
            return result;
        }
        else if (ParseResult::INVALID == parse_help)
        {
            result.first = false;
            return result;
        }

        if (ParseResult::INVALID == middleware_.parse_argument(argc, argv))
        {
            result.first = false;
            return result;
        }
        ParseResult refs_arg = refs_.parse_argument(argc, argv);
        if (ParseResult::VALID == refs_arg)
        {
            struct stat sb;
            if (stat(refs_.value().c_str(), &sb) < 0)
            {
                std::cerr << "Error: reference file '" << refs_.value() << "' does not exist!" << std::endl;
                result.first = false;
                return result;
            }
        }
        else if (ParseResult::INVALID == refs_arg)
        {
            result.first = false;
            return result;
        }

        if (ParseResult::INVALID == verbose_.parse_argument(argc, argv))
        {
            result.first = false;
            return result;
        }
#ifdef UAGENT_DISCOVERY_PROFILE
        if (ParseResult::INVALID == discovery_.parse_argument(argc, argv))
        {
            result.first = false;
            return result;
        }
#endif
#ifdef UAGENT_P2P_PROFILE
        if (ParseResult::INVALID == p2p_.parse_argument(argc, argv))
        {
            result.first = false;
            return result;
        }
#endif
        return result;
    }

    void apply_actions(
            AgentServer<AgentType>* server)
    {
#ifdef UAGENT_DISCOVERY_PROFILE
        if (discovery_.found())
        {
            server->enable_discovery(discovery_.value());
        }
#endif
#ifdef UAGENT_P2P_PROFILE
        if (p2p_.found())
        {
            server->enable_p2p(p2p_.value());
        }
#endif
        if (refs_.found())
        {
            server->load_config_file(refs_.value());
        }
        if (verbose_.found())
        {
            server->set_verbose_level(verbose_.value());
        }
    }

    const std::string get_help() const
    {
        std::stringstream ss;
        ss << "    " << help_.get_help() << std::endl;
        ss << "    " << middleware_.get_help() << std::endl;
        ss << "    " << refs_.get_help() << std::endl;
        ss << "    " << verbose_.get_help() << std::endl;
#ifdef UAGENT_DISCOVERY_PROFILE
        ss << "    " << discovery_.get_help() << std::endl;
#endif
#ifdef UAGENT_P2P_PROFILE
        ss << "    " << p2p_.get_help() << std::endl;
#endif
        return ss.str();
    }

private:
    Argument<dummy_type> help_;
    Argument<std::string> middleware_;
    Argument<std::string> refs_;
    Argument<uint8_t> verbose_;
#ifdef UAGENT_DISCOVERY_PROFILE
    Argument<uint16_t> discovery_;
#endif
#ifdef UAGENT_P2P_PROFILE
    Argument<uint16_t> p2p_;
#endif
};

/*************************************************************************************************
 * Specific arguments for IPvX transports
 *************************************************************************************************/
template <typename AgentType>
class IPvXArgs
{
public:
    IPvXArgs()
        : port_("-p", "--port")
    {
    }

    bool parse(
            int argc,
            char** argv)
    {
        ParseResult parse_port = port_.parse_argument(argc, argv);
        if (ParseResult::VALID != parse_port)
        {
            std::cerr << "Warning: '--port <value>' is required" << std::endl;
        }
        return (ParseResult::VALID == parse_port ? true : false);
    }

    uint16_t port() const
    {
        return port_.value();
    }

    const std::string get_help() const
    {
        std::stringstream ss;
        ss << "    " << port_.get_help() << std::endl;
        return ss.str();
    }

private:
    Argument<uint16_t> port_;
};

#ifndef _WIN32
/*************************************************************************************************
 * Specific arguments for pseudoterminal transports
 *************************************************************************************************/
template <typename AgentType>
class PseudoTerminalArgs
{
public:
    PseudoTerminalArgs()
        : baudrate_("-b", "--baudrate", DEFAULT_BAUDRATE_LEVEL)
    {
    }

    bool parse(
            int argc,
            char** argv)
    {
        bool result = static_cast<bool>(baudrate_.parse_argument(argc, argv));
        return result;
    }

    const std::string baud_rate() const
    {
        return baudrate_.value();
    }

    const std::string get_help() const
    {
        std::stringstream ss;
        ss << "    " << baudrate_.get_help() << std::endl;
        return ss.str();
    }

protected:
    Argument<std::string> baudrate_;
};

/*************************************************************************************************
 * Specific arguments for serial termios transports
 *************************************************************************************************/
template <typename AgentType>
class SerialArgs : public PseudoTerminalArgs<AgentType>
{
public:
    SerialArgs()
        : PseudoTerminalArgs<AgentType>()
        , dev_("-D", "--dev")
    {
    }

    bool parse(
            int argc,
            char** argv)
    {
        if (!PseudoTerminalArgs<AgentType>::parse(argc, argv))
        {
            return false;
        }
        ParseResult parse_dev = dev_.parse_argument(argc, argv);
        if (ParseResult::VALID != parse_dev)
        {
            std::cerr << "Warning: '--dev <value>' is required" << std::endl;
        }
        return (ParseResult::VALID == parse_dev ? true : false);
    }

    const std::string dev() const
    {
        return dev_.value();
    }

    const std::string get_help() const
    {
        std::stringstream ss;
        ss << "    " << dev_.get_help();
        return ss.str();
    }

private:
    Argument<std::string> dev_;
};

/*************************************************************************************************
 * Specific arguments for multi serial termios transports
 *************************************************************************************************/
template <typename AgentType>
class MultiSerialArgs : public PseudoTerminalArgs<AgentType>
{
public:
    MultiSerialArgs()
        : PseudoTerminalArgs<AgentType>()
        , devs_("-D", "--devs")
        , file_("-f", "--file")
    {
    }

    bool parse(
            int argc,
            char** argv)
    {
        if (!PseudoTerminalArgs<AgentType>::parse(argc, argv))
        {
            return false;
        }
        ParseResult parse_devs = devs_.parse_argument(argc, argv);
        ParseResult parse_file = file_.parse_argument(argc, argv);
        if (ParseResult::VALID != parse_devs && ParseResult::VALID != parse_file)
        {
            std::cerr << "Warning: '--devs <values>' or '--file <value>' is required" << std::endl;
        }
        else if(ParseResult::VALID == parse_devs && ParseResult::VALID == parse_file)
        {
            std::cerr << "Warning: '--devs <values>' and '--file <value>' are not allowed" << std::endl;
        }

        return (((ParseResult::VALID == parse_devs) ^ (ParseResult::VALID == parse_file)) ? true : false);
    }

    std::vector<std::string> devs()
    {
        std::vector<std::string> ports;

        if (devs_.found())
        {
            std::istringstream iss(devs_.value());
            for (std::string s; iss >> s; )
            {
                ports.push_back(s);
            }
        }
        else if (file_.found())
        {
            std::string line;
            std::ifstream myfile(file_.value());

            if (myfile.fail())
            {
                std::cerr << "Error opening file: " << strerror(errno) << std::endl;
            }

            while (std::getline(myfile, line))
            {
                ports.push_back(line);
            }
        }

        return ports;
    }

    const std::string get_help() const
    {
        std::stringstream ss;
        ss << "    " << devs_.get_help();
        return ss.str();
    }

private:
    Argument<std::string> devs_;
    Argument<std::string> file_;
};
#endif // _WIN32

/*************************************************************************************************
 * Main argument parser class
 *************************************************************************************************/
template <typename AgentType>
class ArgumentParser
{
public:
    ArgumentParser(
            int argc,
            char** argv,
            TransportKind transport_kind)
        : argc_(argc)
        , argv_(argv)
        , common_args_()
        , ip_args_()
#ifndef _WIN32
        , serial_args_()
        , multiserial_args_()
        , pseudoterminal_args_()
#endif // _WIN32
        , transport_kind_(transport_kind)
        , agent_server_()
    {
    }

    ParseResult parse_arguments()
    {
        if (2 > argc_)
        {
            return ParseResult::INVALID;
        }
        std::pair<bool, bool> common_result = common_args_.parse(argc_, argv_);
        if (common_result.second)
        {
            return ParseResult::HELP;
        }

        bool result = common_result.first;
        switch (transport_kind_)
        {
            case TransportKind::UDP4:
            case TransportKind::UDP6:
            case TransportKind::TCP4:
            case TransportKind::TCP6:
            {
                result &= ip_args_.parse(argc_, argv_);
                break;
            }
#ifndef _WIN32
            case TransportKind::SERIAL:
            {
                result &= serial_args_.parse(argc_, argv_);
                break;
            }
            case TransportKind::MULTISERIAL:
            {
                result &= multiserial_args_.parse(argc_, argv_);
                break;
            }
            case TransportKind::PSEUDOTERMINAL:
            {
                result &= pseudoterminal_args_.parse(argc_, argv_);
                break;
            }
#endif // _WIN32
            case TransportKind::INVALID:
            default:
            {
                result = false;
                break;
            }
        }
        return (result ? ParseResult::VALID : ParseResult::INVALID);
    }

    void show_help()
    {
        utils::usage(argv_[0], false);
        std::stringstream ss;
        ss << std::endl << "Available arguments (per transport):" << std::endl;
        ss << "  * COMMON" << std::endl;
        ss << common_args_.get_help();
        ss << "  * IPvX (udp4, udp6, tcp4, tcp6)" << std::endl;
        ss << ip_args_.get_help();
        ss << "  * SERIAL (serial, multiserial, pseudoterminal)" << std::endl;
#ifndef _WIN32
        ss << pseudoterminal_args_.get_help();
        ss << serial_args_.get_help();
#endif // _WIN32
        ss << std::endl;
        // TODO(@jamoralp): Once documentation is updated with proper CLI section, add here an hyperlink to that section
        std::cout << ss.str();
    }

    bool launch_agent()
    {
        agent_server_.configure_agent_ipv4(ip_args_.port(), utils::get_mw_kind(common_args_.middleware()));

        if (agent_server_.start_agent())
        {
            common_args_.apply_actions(&agent_server_);
            return true;
        }
        else
        {
            std::cerr << "Error while starting IPvX agent!" << std::endl;
        }

        return false;
    }

private:
    int argc_;
    char** argv_;
    CommonArgs<AgentType> common_args_;
    IPvXArgs<AgentType> ip_args_;
#ifndef _WIN32
    SerialArgs<AgentType> serial_args_;
    MultiSerialArgs<AgentType> multiserial_args_;
    PseudoTerminalArgs<AgentType> pseudoterminal_args_;
#endif // _WIN32
    TransportKind transport_kind_;
    AgentServer<AgentType> agent_server_;
};

#ifndef _WIN32
template<> inline bool ArgumentParser<TermiosAgent>::launch_agent()
{
    agent_server_.configure_agent_serial(serial_args_.dev(), serial_args_.baud_rate(), utils::get_mw_kind(common_args_.middleware()));

    if (agent_server_.start_agent())
    {
        common_args_.apply_actions(&agent_server_);
        return true;
    }
    else
    {
        std::cerr << "Error while serial serial agent!" << std::endl;
    }

    return false;
}

template<> inline bool ArgumentParser<MultiTermiosAgent>::launch_agent()
{
    agent_server_.configure_agent_multiserial(multiserial_args_.devs(), multiserial_args_.baud_rate(), utils::get_mw_kind(common_args_.middleware()));

    if (agent_server_.start_agent())
    {
        common_args_.apply_actions(&agent_server_);
        return true;
    }
    else
    {
        std::cerr << "Error while starting multiserial agent!" << std::endl;
    }

    return false;
}

template<> inline bool ArgumentParser<PseudoTerminalAgent>::launch_agent()
{
    agent_server_.configure_agent_pseudoterminal(pseudoterminal_args_.baud_rate(), utils::get_mw_kind(common_args_.middleware()));

    if (agent_server_.start_agent())
    {
        common_args_.apply_actions(&agent_server_);
        return true;
    }
    else
    {
        std::cerr << "Error while starting pseudoterminal agent!" << std::endl;
    }

    return false;
}
#endif // _WIN32

} // namespace parser

/*************************************************************************************************
 * Helper functions to create and launch a microXRCE-DDS agent in a separate thread
 *************************************************************************************************/
template <typename AgentKind>
inline std::thread create_agent_thread(
        int argc,
        char** argv,
#ifndef _WIN32
        eprosima::uxr::agent::TransportKind transport_kind,
        const sigset_t* signals)
#else
        eprosima::uxr::agent::TransportKind transport_kind)
#endif // _WIN32
{
    std::thread agent_thread = std::thread([=]() -> void
    {
        eprosima::uxr::agent::parser::ArgumentParser<AgentKind> parser(argc, argv, transport_kind);

        switch (parser.parse_arguments())
        {
            case parser::ParseResult::INVALID:
            case parser::ParseResult::NOT_FOUND:
            {
                parser::utils::usage(argv[0]);
                break;
            }
            case parser::ParseResult::VALID:
            {
                if (parser.launch_agent())
                {
#ifdef _WIN32
                    /* Waiting until exit. */
                    std::cin.clear();
                    char exit_flag = 0;
                    while ('q' != exit_flag)
                    {
                        std::cin >> exit_flag;
                    }
#else
                    /* Wait for defined signals. */
                    int n_signal = 0;
                    sigwait(signals, &n_signal);
#endif // _WIN32
                }
                break;
            }
            case parser::ParseResult::HELP:
            {
                parser.show_help();
                break;
            }
        }

    });
    return agent_thread;
}


} // namespace agent
} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_UTILS_ARGUMENTPARSER_HPP_
