// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef UXR_AGENT_UTILS_CLI_HPP_
#define UXR_AGENT_UTILS_CLI_HPP_

#ifdef _WIN32
#include <uxr/agent/transport/udp/UDPServerWindows.hpp>
#include <uxr/agent/transport/tcp/TCPServerWindows.hpp>
#else
#include <uxr/agent/transport/udp/UDPServerLinux.hpp>
#include <uxr/agent/transport/tcp/TCPServerLinux.hpp>
#include <uxr/agent/transport/serial/SerialServerLinux.hpp>
#include <uxr/agent/transport/serial/baud_rate_table_linux.h>

#include <termios.h>
#include <fcntl.h>
#endif //_WIN32

#include <uxr/agent/Agent.hpp>

#include <CLI/CLI.hpp>

namespace eprosima{
namespace uxr{
namespace cli {

/*************************************************************************************************
 * Middleware CLI Option
 *************************************************************************************************/
class MiddlewareOpt
{
public:
    MiddlewareOpt(CLI::App& subcommand)
        : kind_{"none"}
        , set_{}
        , cli_opt_{}
    {
#ifdef PROFILE_CED_MIDDLEWARE
        set_.insert("ced");
        kind_ = "ced";
#endif
#ifdef PROFILE_FAST_MIDDLEWARE
        set_.insert("dds");
        kind_ = "dds";
#endif
        cli_opt_ = subcommand.add_set("-m,--middleware", kind_, set_, "Select the kind of Middleware", true);
    }

    eprosima::uxr::Middleware::Kind get_kind() const
    {
#ifdef PROFILE_FAST_MIDDLEWARE
        if ("dds" == kind_)
        {
            return eprosima::uxr::Middleware::Kind::FAST;
        }
#endif
#ifdef PROFILE_CED_MIDDLEWARE
        if ("ced" == kind_)
        {
            return eprosima::uxr::Middleware::Kind::CED;
        }
#endif
        return eprosima::uxr::Middleware::Kind::NONE;
    }

protected:
    std::string kind_;
    std::set<std::string> set_;
    CLI::Option* cli_opt_;
};

/*************************************************************************************************
 * Discovery CLI Option
 *************************************************************************************************/
#ifdef PROFILE_DISCOVERY
class DiscoveryOpt
{
public:
    DiscoveryOpt(CLI::App& subcommand)
        : port_(eprosima::uxr::DISCOVERY_PORT)
        , cli_flag_{subcommand.add_flag("-d,--discovery", "Active the Discovery server")}
        , cli_opt_{subcommand.add_option("--disport", port_, "Select the port for the Discovery server", true)}
    {
        cli_opt_->needs(cli_flag_);
    }

    bool is_enable() const { return bool(*cli_flag_); }
    uint16_t get_port() const { return port_; }

protected:
    uint16_t port_;
    CLI::Option* cli_flag_;
    CLI::Option* cli_opt_;
};
#endif

/*************************************************************************************************
 * P2P CLI Option
 *************************************************************************************************/
#ifdef PROFILE_P2P
class P2POpt
{
public:
    P2POpt(CLI::App& subcommand)
        : port_(0)
        , cli_opt_{subcommand.add_option("--p2p", port_, "Activate the P2P profile using the given port")}
    {
    }

    bool is_enable() const { return bool(*cli_opt_); }
    uint16_t get_port() const { return port_; }

protected:
    uint16_t port_;
    CLI::Option* cli_opt_;
};
#endif

/*************************************************************************************************
 * Baudrate CLI Option
 *************************************************************************************************/
class BaudrateOpt
{
public:
    BaudrateOpt(CLI::App& subcommand)
        : baudrate_("115200")
        , cli_opt_{subcommand.add_option("-b,--baudrate", baudrate_, "Select the baudrate", true)}
    {
    }

    bool is_enable() const { return bool(*cli_opt_); }
    const std::string& get_baudrate() const { return baudrate_; }

protected:
    std::string baudrate_;
    CLI::Option* cli_opt_;
};

/*************************************************************************************************
 * Baudrate CLI Option
 *************************************************************************************************/
class ReferenceOpt
{
public:
    ReferenceOpt(CLI::App& subcommand)
        : file_{}
        , cli_opt_{subcommand.add_option("-r,--refs", file_, "Load a reference file", true)}
    {
        cli_opt_->check(CLI::ExistingFile);
    }

    bool is_enable() const { return bool(*cli_opt_); }
    const std::string& get_file() const { return file_; }

protected:
    std::string file_;
    CLI::Option* cli_opt_;
};

/*************************************************************************************************
 * Logger CLI Option
 *************************************************************************************************/
class VerboseOpt
{
public:
    VerboseOpt(CLI::App& subcommand)
        : level_{4}
        , set_{0, 1, 2, 3, 4, 5, 6}
        , cli_opt_{subcommand.add_set("-v,--verbose", level_, set_, "Select log level from less to more verbose", true)}
    {}

    bool is_enable() const { return bool(*cli_opt_); }
    uint8_t get_level() const { return uint8_t(level_); }

protected:
    uint16_t level_;
    std::set<uint16_t> set_;
    CLI::Option* cli_opt_;
};

/*************************************************************************************************
 * Common CLI Opts
 *************************************************************************************************/
class CommonOpts
{
public:
    CommonOpts(CLI::App& subcommand)
        : middleware_opt_{subcommand}
        , reference_opt_{subcommand}
        , verbose_opt_{subcommand}
#ifdef PROFILE_DISCOVERY
        , discovery_opt_{subcommand}
#endif
#ifdef PROFILE_P2P
        , p2p_opt_{subcommand}
#endif
    {}

    MiddlewareOpt middleware_opt_;
    ReferenceOpt reference_opt_;
    VerboseOpt verbose_opt_;
#ifdef PROFILE_DISCOVERY
    DiscoveryOpt discovery_opt_;
#endif
#ifdef PROFILE_P2P
    P2POpt p2p_opt_;
#endif
};

/*************************************************************************************************
 * Exit Subcommand
 *************************************************************************************************/
class ExitSubcommand
{
public:
    ExitSubcommand(
            CLI::App& app)
        : exit_subcommand_{app.add_subcommand("q", "Exit")}
    {
        exit_subcommand_->callback(std::bind(&ExitSubcommand::exit, this));
    }

private:
    void exit()
    {
        std::exit(EXIT_SUCCESS);
    }

private:
    CLI::App* exit_subcommand_;
};

/*************************************************************************************************
 * Server Subcommand
 *************************************************************************************************/
class ServerSubcommand
{
public:
    ServerSubcommand(
            CLI::App& app,
            const std::string& name,
            const std::string& description,
            const CommonOpts& common_opts)
        : cli_subcommand_{app.add_subcommand(name, description)}
        , opts_ref_{common_opts}
    {
        cli_subcommand_->callback(std::bind(&ServerSubcommand::server_callback, this));
    }

    virtual ~ServerSubcommand() = default;

private:
    void server_callback()
    {
        std::cout << "Enter 'q' for exit" << std::endl;
        if (launch_server())
        {
#ifdef PROFILE_DISCOVERY
            if (opts_ref_.discovery_opt_.is_enable())
            {
                server_->enable_discovery(opts_ref_.discovery_opt_.get_port());
            }
#endif

#ifdef PROFILE_P2P
            if ((eprosima::uxr::Middleware::Kind::CED == opts_ref_.middleware_opt_.get_kind())
                && opts_ref_.p2p_opt_.is_enable())
            {
                server_->enable_p2p(opts_ref_.p2p_opt_.get_port());
            }
#endif
            if (opts_ref_.reference_opt_.is_enable())
            {
                Agent::load_config_file(opts_ref_.reference_opt_.get_file());
            }

            if (opts_ref_.verbose_opt_.is_enable())
            {
                Agent::set_verbose_level(opts_ref_.verbose_opt_.get_level());
            }
        }
    }

    virtual bool launch_server() = 0;

protected:
    std::unique_ptr<eprosima::uxr::Server> server_;
    CLI::App* cli_subcommand_;
    const CommonOpts& opts_ref_;
};


/*************************************************************************************************
 * UDP Subcommand
 *************************************************************************************************/
class UDPSubcommand : public ServerSubcommand
{
public:
    UDPSubcommand(CLI::App& app)
        : ServerSubcommand{app, "udp", "Launch a UDP server", common_opts_}
        , cli_opt_{cli_subcommand_->add_option("-p,--port", port_, "Select the port")}
        , common_opts_{*cli_subcommand_}
    {
        cli_opt_->required(true);
    }

    ~UDPSubcommand() final = default;

private:
    bool launch_server()
    {
        server_.reset(new eprosima::uxr::UDPServer(port_, common_opts_.middleware_opt_.get_kind()));
        return server_->run();
    }

private:
    uint16_t port_;
    CLI::Option* cli_opt_;
    CommonOpts common_opts_;
};

/*************************************************************************************************
 * TCP Subcommand
 *************************************************************************************************/
class TCPSubcommand : public ServerSubcommand
{
public:
    TCPSubcommand(CLI::App& app)
        : ServerSubcommand{app, "tcp", "Launch a TCP server", common_opts_}
        , cli_opt_{cli_subcommand_->add_option("-p,--port", port_, "Select the port")}
        , common_opts_{*cli_subcommand_}
    {
        cli_opt_->required(true);
    }

    ~TCPSubcommand() final = default;

private:
    bool launch_server()
    {
        server_.reset(new eprosima::uxr::TCPServer(port_, common_opts_.middleware_opt_.get_kind()));
        return server_->run();
    }

private:
    uint16_t port_;
    CLI::Option* cli_opt_;
    CommonOpts common_opts_;
};

/*************************************************************************************************
 * Serial Subcommand
 *************************************************************************************************/
#ifndef _WIN32
class SerialSubcommand : public ServerSubcommand
{
public:
    SerialSubcommand(CLI::App& app)
        : ServerSubcommand{app, "serial", "Launch a Serial server", common_opts_}
        , cli_opt_{cli_subcommand_->add_option("-d,--dev", dev_, "Select the serial device")}
        , baudrate_opt_{*cli_subcommand_}
        , common_opts_{*cli_subcommand_}
    {
        cli_opt_->required(true);
        cli_opt_->check(CLI::ExistingFile);
    }

private:
    bool launch_server() final
    {
        bool rv = false;
        int fd = open(dev_.c_str(), O_RDWR | O_NOCTTY);
        if (0 < fd)
        {
            struct termios attr;
            memset(&attr, 0, sizeof(attr));
            if (0 == tcgetattr(fd, &attr))
            {
                /* Setting CONTROL OPTIONS. */
                attr.c_cflag |= unsigned(CREAD);    // Enable read.
                attr.c_cflag |= unsigned(CLOCAL);   // Set local mode.
                attr.c_cflag &= unsigned(~PARENB);  // Disable parity.
                attr.c_cflag &= unsigned(~CSTOPB);  // Set one stop bit.
                attr.c_cflag &= unsigned(~CSIZE);   // Mask the character size bits.
                attr.c_cflag |= unsigned(CS8);      // Set 8 data bits.
                attr.c_cflag &= unsigned(~CRTSCTS); // Disable hardware flow control.

                /* Setting LOCAL OPTIONS. */
                attr.c_lflag &= unsigned(~ICANON);  // Set non-canonical input.
                attr.c_lflag &= unsigned(~ECHO);    // Disable echoing of input characters.
                attr.c_lflag &= unsigned(~ECHOE);   // Disable echoing the erase character.
                attr.c_lflag &= unsigned(~ISIG);    // Disable SIGINTR, SIGSUSP, SIGDSUSP and SIGQUIT signals.

                /* Setting INPUT OPTIONS. */
                attr.c_iflag &= unsigned(~IXON);    // Disable output software flow control.
                attr.c_iflag &= unsigned(~IXOFF);   // Disable input software flow control.
                attr.c_iflag &= unsigned(~INPCK);   // Disable parity check.
                attr.c_iflag &= unsigned(~ISTRIP);  // Disable strip parity bits.
                attr.c_iflag &= unsigned(~IGNBRK);  // No ignore break condition.
                attr.c_iflag &= unsigned(~IGNCR);   // No ignore carrier return.
                attr.c_iflag &= unsigned(~INLCR);   // No map NL to CR.
                attr.c_iflag &= unsigned(~ICRNL);   // No map CR to NL.

                /* Setting OUTPUT OPTIONS. */
                attr.c_oflag &= unsigned(~OPOST);   // Set raw output.

                /* Setting OUTPUT CHARACTERS. */
                attr.c_cc[VMIN] = 10;
                attr.c_cc[VTIME] = 1;

                /* Get baudrate. */
                speed_t baudrate = getBaudRate(baudrate_opt_.get_baudrate().c_str());

                /* Setting BAUD RATE. */
                cfsetispeed(&attr, baudrate);
                cfsetospeed(&attr, baudrate);

                if (0 == tcsetattr(fd, TCSANOW, &attr))
                {
                    server_.reset(new eprosima::uxr::SerialServer(fd, 0, common_opts_.middleware_opt_.get_kind()));
                    rv = server_->run();
                }
            }
        }
        return rv;
    }

private:
    std::string dev_;
    CLI::Option* cli_opt_;
    BaudrateOpt baudrate_opt_;
    CommonOpts common_opts_;
};

/*************************************************************************************************
 * Pseudo-Serial Subcommand
 *************************************************************************************************/
class PseudoSerialSubcommand : public ServerSubcommand
{
public:
    PseudoSerialSubcommand(CLI::App& app)
        : ServerSubcommand{app, "pseudo-serial", "Launch a Pseudo-Serial server", common_opts_}
        , baudrate_opt_{*cli_subcommand_}
        , common_opts_{*cli_subcommand_}
    {}

private:
    bool launch_server() final
    {
        bool rv = false;

        /* Open pseudo-terminal. */
        char* dev = nullptr;
        int fd = posix_openpt(O_RDWR | O_NOCTTY);
        if (-1 != fd)
        {
            if (grantpt(fd) == 0 && unlockpt(fd) == 0 && (dev = ptsname(fd)))
            {
                struct termios attr;
                tcgetattr(fd, &attr);
                cfmakeraw(&attr);
                tcflush(fd, TCIOFLUSH);
                tcsetattr(fd, TCSANOW, &attr);

                /* Get baudrate. */
                speed_t baudrate = getBaudRate(baudrate_opt_.get_baudrate().c_str());

                /* Setting BAUD RATE. */
                cfsetispeed(&attr, baudrate);
                cfsetospeed(&attr, baudrate);

                /* Log. */
                std::cout << "Pseudo-Serial device opend at " << dev << std::endl;

                /* Run server. */
                server_.reset(new eprosima::uxr::SerialServer(fd, 0x00, common_opts_.middleware_opt_.get_kind()));
                rv = server_->run();
            }
        }

        return rv;
    }

private:
    BaudrateOpt baudrate_opt_;
    CommonOpts common_opts_;
};
#endif

} // cli
} // uxr
} // eprosima

#endif // UXR_AGENT_UTILS_CLI_HPP_
