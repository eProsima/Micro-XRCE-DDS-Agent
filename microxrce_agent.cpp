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

#include <CLI/CLI.hpp>

#include <iostream>
#include <string>
#include <sstream>
#include <limits>
#include <iterator>

/*************************************************************************************************
 * Middleware CLI Option
 *************************************************************************************************/
class MiddlewareOpt
{
public:
    MiddlewareOpt(CLI::App& subcommand)
        : kind_{}
        , set_{}
        , cli_opt_{}
    {
#ifdef PROFILE_FAST_MIDDLEWARE
        set_.insert("dds");
#endif
#ifdef PROFILE_CED_MIDDLEWARE
        set_.insert("ced");
#endif
        kind_ = *(set_.rbegin());
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
        , cli_opt_{subcommand.add_option("--discovery-port", port_, "Select the port for the Discovery server", true)}
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

    bool is_enable() const { return bool(cli_opt_); }
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

    bool is_enable() const { return bool(cli_opt_); }
    const std::string& get_baudrate() const { return baudrate_; }

protected:
    std::string baudrate_;
    CLI::Option* cli_opt_;
};

/*************************************************************************************************
 * UDP CLI Subcommand
 *************************************************************************************************/
class UDPSubcommand
{
public:
    UDPSubcommand(CLI::App& app)
        : cli_subcommand_{app.add_subcommand("udp", "Launch a UDP server")}
        , cli_opt_{cli_subcommand_->add_option("-p,--port", port_, "Select the port")}
        , middleware_opt_{*cli_subcommand_}
#ifdef PROFILE_DISCOVERY
        , discovery_opt_{*cli_subcommand_}
#endif
#ifdef PROFILE_P2P
        , p2p_opt_{*cli_subcommand_}
#endif
    {
        cli_subcommand_->callback(std::bind(&UDPSubcommand::launch_server, this));
        cli_opt_->required(true);
    }

private:
    void launch_server()
    {
        server_.reset(new eprosima::uxr::UDPServer(port_, middleware_opt_.get_kind()));
        server_->run();

        if (discovery_opt_.is_enable())
        {
            server_->enable_discovery(discovery_opt_.get_port());
        }

#ifdef PROFILE_P2P
        if (eprosima::uxr::Middleware::Kind::CED == middleware_opt_.get_kind() && p2p_opt_.is_enable())
        {
            server_->enable_p2p(p2p_opt_.get_port());
        }
#endif
    }

private:
    uint16_t port_;
    std::unique_ptr<eprosima::uxr::Server> server_;
    CLI::App* cli_subcommand_;
    CLI::Option* cli_opt_;
    MiddlewareOpt middleware_opt_;
#ifdef PROFILE_DISCOVERY
    DiscoveryOpt discovery_opt_;
#endif
#ifdef PROFILE_P2P
    P2POpt p2p_opt_;
#endif
};

/*************************************************************************************************
 * TCP CLI Subcommand
 *************************************************************************************************/
class TCPSubcommand
{
public:
    TCPSubcommand(CLI::App& app)
        : cli_subcommand_{app.add_subcommand("tcp", "Launch a TCP server")}
        , cli_opt_{cli_subcommand_->add_option("-p,--port", port_, "Select the port")}
        , middleware_opt_{*cli_subcommand_}
#ifdef PROFILE_DISCOVERY
        , discovery_opt_{*cli_subcommand_}
#endif
#ifdef PROFILE_P2P
        , p2p_opt_{*cli_subcommand_}
#endif
    {
        cli_subcommand_->callback(std::bind(&TCPSubcommand::launch_server, this));
        cli_opt_->required(true);
    }

private:
    void launch_server()
    {
        server_.reset(new eprosima::uxr::TCPServer(port_, middleware_opt_.get_kind()));
        server_->run();

        if (discovery_opt_.is_enable())
        {
            server_->enable_discovery(discovery_opt_.get_port());
        }

#ifdef PROFILE_P2P
        if (eprosima::uxr::Middleware::Kind::CED == middleware_opt_.get_kind() && p2p_opt_.is_enable())
        {
            server_->enable_p2p(p2p_opt_.get_port());
        }
#endif
    }

private:
    uint16_t port_;
    std::unique_ptr<eprosima::uxr::Server> server_;
    CLI::App* cli_subcommand_;
    CLI::Option* cli_opt_;
    MiddlewareOpt middleware_opt_;
#ifdef PROFILE_DISCOVERY
    DiscoveryOpt discovery_opt_;
#endif
#ifdef PROFILE_P2P
    P2POpt p2p_opt_;
#endif
};

/*************************************************************************************************
 * Serial CLI Subcommand
 *************************************************************************************************/
class SerialSubcommand
{
public:
    SerialSubcommand(CLI::App& app)
        : cli_subcommand_{app.add_subcommand("serial", "Launch a Serial server")}
        , cli_opt_{cli_subcommand_->add_option("-d,--dev", dev_, "Select the serial device")}
        , middleware_opt_{*cli_subcommand_}
        , baudrate_opt_{*cli_subcommand_}
#ifdef PROFILE_P2P
        , p2p_opt_{*cli_subcommand_}
#endif
    {
        cli_subcommand_->callback(std::bind(&SerialSubcommand::launch_server, this));
        cli_opt_->required(true);
    }

private:
    void launch_server()
    {
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
                    server_.reset(new eprosima::uxr::SerialServer(fd, 0, middleware_opt_.get_kind()));
                    server_->run();

#ifdef PROFILE_P2P
                    if (eprosima::uxr::Middleware::Kind::CED == middleware_opt_.get_kind() && p2p_opt_.is_enable())
                    {
                        server_->enable_p2p(p2p_opt_.get_port());
                    }
#endif
                }
            }
        }
    }

private:
    std::string dev_;
    std::unique_ptr<eprosima::uxr::Server> server_;
    CLI::App* cli_subcommand_;
    CLI::Option* cli_opt_;
    MiddlewareOpt middleware_opt_;
    BaudrateOpt baudrate_opt_;
#ifdef PROFILE_P2P
    P2POpt p2p_opt_;
#endif
};

/*************************************************************************************************
 * Pseudo-Serial CLI Subcommand
 *************************************************************************************************/
class PseudoSerialSubcommand
{
public:
    PseudoSerialSubcommand(CLI::App& app)
        : cli_subcommand_{app.add_subcommand("pseudo-serial", "Launch a Pseudo-Serial server")}
        , middleware_opt_{*cli_subcommand_}
        , baudrate_opt_{*cli_subcommand_}
#ifdef PROFILE_P2P
        , p2p_opt_{*cli_subcommand_}
#endif
    {
        cli_subcommand_->callback(std::bind(&PseudoSerialSubcommand::launch_server, this));
    }

private:
    void launch_server()
    {
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
            }
        }

        server_.reset(new eprosima::uxr::SerialServer(fd, 0x00, middleware_opt_.get_kind()));
        server_->run();

#ifdef PROFILE_P2P
        if (eprosima::uxr::Middleware::Kind::CED == middleware_opt_.get_kind() && p2p_opt_.is_enable())
        {
            server_->enable_p2p(p2p_opt_.get_port());
        }
#endif
    }

private:
    std::unique_ptr<eprosima::uxr::Server> server_;
    CLI::App* cli_subcommand_;
    MiddlewareOpt middleware_opt_;
    BaudrateOpt baudrate_opt_;
#ifdef PROFILE_P2P
    P2POpt p2p_opt_;
#endif
};

int main(int argc, char** argv)
{
    std::cout << "Enter 'q' for exit" << std::endl;

    /* CLI application. */
    CLI::App app("eProsima Micro XRCE-DDS Agent");
    app.require_subcommand(0, 1);
    app.get_formatter()->column_width(4);

    /* CLI subcommands. */
    UDPSubcommand udp_subcommand(app);
    TCPSubcommand tcp_subcommand(app);
    SerialSubcommand serial_subcommand(app);
    PseudoSerialSubcommand pseudo_serial_subcommand(app);

    /* CLI parse. */
    CLI11_PARSE(app, argc, argv);

    /* Waiting until exit. */
    std::cin.clear();
    char exit_flag = 0;
    while ('q' != exit_flag)
    {
        std::cin >> exit_flag;
    }

    return 0;
}
