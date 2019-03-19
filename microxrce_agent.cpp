// Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

class MiddlewareOpt
{
public:
    MiddlewareOpt(CLI::App& subcommand)
        : kind_("dds")
        , cli_opt_{subcommand.add_set("-m,--middleware", kind_, {"dds", "ced"}, "Select the kind of Middleware", true)}
    {}

    eprosima::uxr::Middleware::Kind get_kind() const
    {
        eprosima::uxr::Middleware::Kind rv = eprosima::uxr::Middleware::Kind::FAST;
        if ("ced" == kind_)
        {
            rv = eprosima::uxr::Middleware::Kind::CED;
        }
        return rv;
    }

protected:
    std::string kind_;
    CLI::Option* cli_opt_;
};

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

class BaudrateOpt
{
public:
    BaudrateOpt(CLI::App& subcommand)
        : baudrate_(115200)
        , set_{0, 50, 75, 110, 134, 150, 200, 300, 600, 1200}
        , cli_opt_{subcommand.add_option("-b,--baudrate", baudrate_, "Select the baudrate", true)}
    {
    }

    bool is_enable() const { return bool(cli_opt_); }
    speed_t get_baudrate() const { return baudrate_; }

protected:
    speed_t baudrate_;
    std::set<speed_t> set_;
    CLI::Option* cli_opt_;
};

class UDPSubcommand
{
public:
    UDPSubcommand(CLI::App& app)
        : cli_subcommand_{app.add_subcommand("udp", "Launch a UDP server")}
        , cli_opt_{cli_subcommand_->add_option("-p,--port", port_, "Select the port")}
        , middleware_opt_{*cli_subcommand_}
        , discovery_opt_{*cli_subcommand_}
        , p2p_opt_{*cli_subcommand_}
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

        if (eprosima::uxr::Middleware::Kind::CED == middleware_opt_.get_kind() && p2p_opt_.is_enable())
        {
            server_->enable_p2p(p2p_opt_.get_port());
        }
    }

private:
    uint16_t port_;
    std::unique_ptr<eprosima::uxr::Server> server_;
    CLI::App* cli_subcommand_;
    CLI::Option* cli_opt_;
    MiddlewareOpt middleware_opt_;
    DiscoveryOpt discovery_opt_;
    P2POpt p2p_opt_;
};

/*************************************************************************************************
 * TCP Subcommand
 *************************************************************************************************/
class TCPSubcommand
{
public:
    TCPSubcommand(CLI::App& app)
        : cli_subcommand_{app.add_subcommand("tcp", "Launch a TCP server")}
        , cli_opt_{cli_subcommand_->add_option("-p,--port", port_, "Select the port")}
        , middleware_opt_{*cli_subcommand_}
        , discovery_opt_{*cli_subcommand_}
        , p2p_opt_{*cli_subcommand_}
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

        if (eprosima::uxr::Middleware::Kind::CED == middleware_opt_.get_kind() && p2p_opt_.is_enable())
        {
            server_->enable_p2p(p2p_opt_.get_port());
        }
    }

private:
    uint16_t port_;
    std::unique_ptr<eprosima::uxr::Server> server_;
    CLI::App* cli_subcommand_;
    CLI::Option* cli_opt_;
    MiddlewareOpt middleware_opt_;
    DiscoveryOpt discovery_opt_;
    P2POpt p2p_opt_;
};

class SerialSubcommand
{
public:
    SerialSubcommand(CLI::App& app)
        : cli_subcommand_{app.add_subcommand("serial", "Launch a Serial server")}
        , cli_opt_{cli_subcommand_->add_option("-d,--dev", dev_, "Select the serial device")}
        , middleware_opt_{*cli_subcommand_}
        , baudrate_opt_{*cli_subcommand_}
        , p2p_opt_{*cli_subcommand_}
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

                /* Setting BAUD RATE. */
                cfsetispeed(&attr, B115200);
                cfsetospeed(&attr, B115200);

                if (0 == tcsetattr(fd, TCSANOW, &attr))
                {
                    server_.reset(new eprosima::uxr::SerialServer(fd, 0, middleware_opt_.get_kind()));
                    server_->run();

                    if (eprosima::uxr::Middleware::Kind::CED == middleware_opt_.get_kind() && p2p_opt_.is_enable())
                    {
                        server_->enable_p2p(p2p_opt_.get_port());
                    }
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
    P2POpt p2p_opt_;
};

class PseudoSerialSubcommand
{
public:
    PseudoSerialSubcommand(CLI::App& app)
        : cli_subcommand_{app.add_subcommand("pseudo-serial", "Launch a Pseudo-Serial server")}
        , middleware_opt_{*cli_subcommand_}
        , baudrate_opt_{*cli_subcommand_}
        , p2p_opt_{*cli_subcommand_}
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
                cfsetispeed(&attr, B115200);
                cfsetospeed(&attr, B115200);
                tcsetattr(fd, TCSANOW, &attr);

                /* Log. */
                std::cout << "Pseudo-Serial device opend at " << dev << std::endl;
            }
        }

        server_.reset(new eprosima::uxr::SerialServer(fd, 0x00, middleware_opt_.get_kind()));
        server_->run();

        if (eprosima::uxr::Middleware::Kind::CED == middleware_opt_.get_kind() && p2p_opt_.is_enable())
        {
            server_->enable_p2p(p2p_opt_.get_port());
        }
    }

private:
    std::unique_ptr<eprosima::uxr::Server> server_;
    CLI::App* cli_subcommand_;
    MiddlewareOpt middleware_opt_;
    BaudrateOpt baudrate_opt_;
    P2POpt p2p_opt_;
};

//void showMiddlewareHelp()
//{
//    std::cout << "[--middleware <";
//#ifdef PROFILE_FAST_MIDDLEWARE
//    std::cout << "dds|";
//#endif
//#ifdef PROFILE_CED_MIDDLEWARE
//    std::cout << "ced|";
//#endif
//    std::cout << ">] ";
//}
//
//void showDiscoveryHelp()
//{
//#ifdef PROFILE_DISCOVERY
//    std::cout << "[--discovery [<discovery_port>]] ";
//#endif
//}
//
//void showP2PHelp()
//{
//#ifdef PROFILE_P2P
//    std::cout << "[--p2p <p2p_port>] ";
//#endif
//}
//
//void showHelp()
//{
//    std::cout << "Usage: program <command>" << std::endl;
//    std::cout << "List of commands:" << std::endl;
//
//    /*
//     * UDP commands.
//     */
//    std::cout << "    --udp <local_port>     ";
//    showMiddlewareHelp();
//    showDiscoveryHelp();
//    showP2PHelp();
//    std::cout << std::endl;
//
//    /*
//     * TCP commands.
//     */
//    std::cout << "    --tcp <local_port>     ";
//    showMiddlewareHelp();
//    showDiscoveryHelp();
//    showP2PHelp();
//    std::cout << std::endl;
//
//    /*
//     * Serial commands.
//     */
//#ifndef _WIN32
//    std::cout << "    --serial <device_name> ";
//    showMiddlewareHelp();
//    std::cout << std::endl;
//    std::cout << "    --pseudo-serial        ";
//    showMiddlewareHelp();
//    std::cout << std::endl;
//#endif
//}
//
//void initializationError()
//{
//    std::cout << "Error: Invalid arguments." << std::endl;
//    showHelp();
//    std::exit(EXIT_FAILURE);
//}
//
//uint16_t parsePort(const std::string& str_port)
//{
//    uint16_t valid_port = 0;
//    try
//    {
//        int port = std::stoi(str_port);
//        if(port > (std::numeric_limits<uint16_t>::max)())
//        {
//            std::cout << "Error: port number '" << port << "out of range." << std::endl;
//            initializationError();
//        }
//        valid_port = uint16_t(port);
//    }
//    catch (const std::invalid_argument& )
//    {
//        initializationError();
//    }
//    return valid_port;
//}
//
//void parseMiddleware(
//        const std::vector<std::string>& cl,
//        uint8_t& cl_counter,
//        eprosima::uxr::MiddlewareKind& middleware_kind)
//{
//    if (cl_counter <= cl.size())
//    {
//        if (("--middleware" == cl[cl_counter - 1]))
//        {
//            ++cl_counter;
//            if (cl_counter <= cl.size())
//            {
//                bool middleware_set = false;
//#ifdef PROFILE_CED_MIDDLEWARE
//                if ("ced" == cl[cl_counter - 1])
//                {
//                    middleware_kind = eprosima::uxr::CED_MIDDLEWARE;
//                    middleware_set = true;
//                }
//#endif
//#ifdef PROFILE_FAST_MIDDLEWARE
//                if("dds" == cl[cl_counter - 1])
//                {
//                    middleware_kind = eprosima::uxr::FAST_MIDDLEWARE;
//                    middleware_set = true;
//                }
//#endif
//                if (!middleware_set)
//                {
//                    initializationError();
//                }
//                ++cl_counter;
//            }
//        }
//    }
//}
//
//#ifdef PROFILE_DISCOVERY
//void parseDiscovery(
//        const std::vector<std::string>& cl,
//        uint8_t& cl_counter,
//        eprosima::uxr::Server* server)
//{
//    if(cl_counter <= cl.size())
//    {
//        if(("--discovery" == cl[cl_counter - 1]))
//        {
//            ++cl_counter;
//            if (cl_counter <= cl.size() && "--" != cl[cl_counter - 1].substr(0, 2))
//            {
//                server->enable_discovery(parsePort(cl[cl_counter - 1]));
//                ++cl_counter;
//            }
//            else
//            {
//                server->enable_discovery();
//            }
//        }
//    }
//}
//#endif
//
//#ifdef PROFILE_P2P
//void parseP2P(
//        const std::vector<std::string>& cl,
//        uint8_t& cl_counter,
//        eprosima::uxr::Server* server)
//{
//    if (cl_counter <= cl.size())
//    {
//        if(("--p2p" == cl[cl_counter - 1]))
//        {
//            ++cl_counter;
//            if (cl_counter <= cl.size())
//            {
//                server->enable_p2p(parsePort(cl[cl_counter - 1]));
//            }
//            else
//            {
//                initializationError();
//            }
//            ++cl_counter;
//        }
//    }
//}
//#endif

int main(int argc, char** argv)
{
//    std::cout << "Enter 'q' for exit" << std::endl;
//
//    eprosima::uxr::Server* server = nullptr;
//    std::vector<std::string> cl(0);
//    uint8_t cl_counter = 1;
//
//    if (1 == argc)
//    {
//        showHelp();
//        std::cout << std::endl;
//        std::cout << "Enter command: ";
//
//        std::string raw_cl;
//        std::getline(std::cin, raw_cl);
//        std::istringstream iss(raw_cl);
//        cl.insert(cl.begin(), std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>());
//        std::cout << raw_cl << std::endl;
//    }
//    else
//    {
//        for (int i = 1; i < argc; ++i)
//        {
//            cl.push_back(argv[i]);
//        }
//    }
//
//    if((1 == cl.size()) && (("-h" == cl[0]) || ("--help" == cl[0])))
//    {
//        showHelp();
//    }
//    else if((2 <= cl.size()) && ("--udp" == cl[0]))
//    {
//        uint16_t port = parsePort(cl[1]);
//        cl_counter += 2;
//
//        /* Get middleware. */
//        eprosima::uxr::MiddlewareKind middleware_kind = eprosima::uxr::FAST_MIDDLEWARE;
//        parseMiddleware(cl, cl_counter, middleware_kind);
//
//        /* Run server. */
//        server = new eprosima::uxr::UDPServer(port, middleware_kind);
//        if (!server->run())
//        {
//            return 1;
//        }
//
//#ifdef PROFILE_DISCOVERY
//        /* Get discovery. */
//        parseDiscovery(cl, cl_counter, server);
//#endif
//
//#ifdef PROFILE_P2P
//        /* Get p2p. */
//        parseP2P(cl, cl_counter, server);
//#endif
//    }
//    else if((2 <= cl.size()) && ("--tcp" == cl[0]))
//    {
//        uint16_t port = parsePort(cl[1]);
//        cl_counter += 2;
//
//        /* Get middleware. */
//        eprosima::uxr::MiddlewareKind middleware_kind = eprosima::uxr::FAST_MIDDLEWARE;
//        parseMiddleware(cl, cl_counter, middleware_kind);
//
//        /* Run server. */
//        server = new eprosima::uxr::TCPServer(port, middleware_kind);
//        if (server->run())
//        {
//            std::cout << "--> OK: TCP Agent running at port " << port << std::endl;
//        }
//        else
//        {
//            std::cerr << "--> ERROR: failed to start TCP Agent" << std::endl;
//            return 1;
//        }
//
//#ifdef PROFILE_DISCOVERY
//        /* Get discovery. */
//        parseDiscovery(cl, cl_counter, server);
//#endif
//#ifdef PROFILE_P2P
//        /* Get p2p. */
//        parseP2P(cl, cl_counter, server);
//#endif
//    }
//#ifndef _WIN32
//    else if((2 == cl.size()) && ("--serial" == cl[0]))
//    {
//        /* Open serial device. */
//        int fd = open(cl[1].c_str(), O_RDWR | O_NOCTTY);
//        if (0 < fd)
//        {
//            struct termios tty_config;
//            memset(&tty_config, 0, sizeof(tty_config));
//            if (0 == tcgetattr(fd, &tty_config))
//            {
//                /* Setting CONTROL OPTIONS. */
//                tty_config.c_cflag |= CREAD;    // Enable read.
//                tty_config.c_cflag |= CLOCAL;   // Set local mode.
//                tty_config.c_cflag &= ~PARENB;  // Disable parity.
//                tty_config.c_cflag &= ~CSTOPB;  // Set one stop bit.
//                tty_config.c_cflag &= ~CSIZE;   // Mask the character size bits.
//                tty_config.c_cflag |= CS8;      // Set 8 data bits.
//                tty_config.c_cflag &= ~CRTSCTS; // Disable hardware flow control.
//
//                /* Setting LOCAL OPTIONS. */
//                tty_config.c_lflag &= ~ICANON;  // Set non-canonical input.
//                tty_config.c_lflag &= ~ECHO;    // Disable echoing of input characters.
//                tty_config.c_lflag &= ~ECHOE;   // Disable echoing the erase character.
//                tty_config.c_lflag &= ~ISIG;    // Disable SIGINTR, SIGSUSP, SIGDSUSP and SIGQUIT signals.
//
//                /* Setting INPUT OPTIONS. */
//                tty_config.c_iflag &= ~IXON;    // Disable output software flow control.
//                tty_config.c_iflag &= ~IXOFF;   // Disable input software flow control.
//                tty_config.c_iflag &= ~INPCK;   // Disable parity check.
//                tty_config.c_iflag &= ~ISTRIP;  // Disable strip parity bits.
//                tty_config.c_iflag &= ~IGNBRK;  // No ignore break condition.
//                tty_config.c_iflag &= ~IGNCR;   // No ignore carrier return.
//                tty_config.c_iflag &= ~INLCR;   // No map NL to CR.
//                tty_config.c_iflag &= ~ICRNL;   // No map CR to NL.
//
//                /* Setting OUTPUT OPTIONS. */
//                tty_config.c_oflag &= ~OPOST;   // Set raw output.
//
//                /* Setting OUTPUT CHARACTERS. */
//                tty_config.c_cc[VMIN] = 10;
//                tty_config.c_cc[VTIME] = 1;
//
//                /* Setting BAUD RATE. */
//                cfsetispeed(&tty_config, B115200);
//                cfsetospeed(&tty_config, B115200);
//
//                if (0 == tcsetattr(fd, TCSANOW, &tty_config))
//                {
//                    /* Get middleware. */
//                    eprosima::uxr::MiddlewareKind middleware_kind = eprosima::uxr::FAST_MIDDLEWARE;
//                    parseMiddleware(cl, cl_counter, middleware_kind);
//
//                    server = new eprosima::uxr::SerialServer(fd, 0, middleware_kind);
//                    if (server->run())
//                    {
//                        std::cout << "--> Serial Agent running at device " << cl[1] << std::endl;
//                    }
//                    else
//                    {
//                        std::cerr << "--> Serial Agent initialization ERROR" << std::endl;
//                        return 1;
//                    }
//                }
//            }
//        }
//    }
//    else if ((1 == cl.size()) && ("--pseudo-serial" == cl[0]))
//    {
//        /* Open pseudo-terminal. */
//        char* dev = nullptr;
//        int fd = posix_openpt(O_RDWR | O_NOCTTY);
//        if (-1 != fd)
//        {
//            if (grantpt(fd) == 0 && unlockpt(fd) == 0 && (dev = ptsname(fd)))
//            {
//                struct termios attr;
//                tcgetattr(fd, &attr);
//                cfmakeraw(&attr);
//                tcflush(fd, TCIOFLUSH);
//                cfsetispeed(&attr, B115200);
//                cfsetospeed(&attr, B115200);
//                tcsetattr(fd, TCSANOW, &attr);
//            }
//        }
//
//        /* Get middleware. */
//        eprosima::uxr::MiddlewareKind middleware_kind = eprosima::uxr::FAST_MIDDLEWARE;
//        parseMiddleware(cl, cl_counter, middleware_kind);
//
//        server = new eprosima::uxr::SerialServer(fd, 0x00, middleware_kind);
//        if (server->run())
//        {
//            std::cout << "--> Pseudo-Serial Agent running at device " << dev << std::endl;
//        }
//        else
//        {
//            std::cerr << "--> Pseudo-Serial Agent initialization ERROR" << std::endl;
//            return 1;
//        }
//    }
//#endif
//    else
//    {
//        initializationError();
//    }
//
//    /* Waiting until exit. */
//    std::cin.clear();
//    char exit_flag = 0;
//    while ('q' != exit_flag)
//    {
//        std::cin >> exit_flag;
//    }
//    if (server)
//    {
//        server->stop();
//        delete server;
//    }

    CLI::App app("eProsima Micro XRCE-DDS Agent");
    app.require_subcommand(0, 1);
    app.get_formatter()->column_width(4);

    UDPSubcommand udp_subcommand(app);
    TCPSubcommand tcp_subcommand(app);
    SerialSubcommand serial_subcommand(app);
    PseudoSerialSubcommand pseudo_serial_subcommand(app);

//    MiddlewareOpt middleware_opt(*udp);
//    DiscoveryOpt discovery_opt(*udp);
//    P2POpt p2p_opt(*udp);

//    std::string middleware_kind = "dds";
//    CLI::Option* middleware = udp->add_set("-m,--middleware", middleware_kind, {"dds", "ced"}, "Select the Middleware kind", true);

//    CLI::Option* discovery_flag = udp->add_flag("-d,--discovery")->description("Activate the Discovery server over the port 7400 by default");
//    udp->add_option("-v,--discovery-port", discovery_port, "Select the port to be used by the Discovery Server", true)->needs(discovery_flag);
//    CLI::Option* discovery_flag = udp->add_flag("-d,--discovery", "Activate the Discovery server");
//    CLI::Option* discovery_opt = udp->add_option("--discovery-port", discovery_port, "Select the port for the Discovery server", true);
//    discovery_opt->needs(discovery_flag);

//    CLI::Option* p2p = udp->add_option("-t,--p2p", p2p_port, "Activate the P2P profile using the given port");
//    p2p->needs(middleware);

    CLI11_PARSE(app, argc, argv);

    return 0;
}
