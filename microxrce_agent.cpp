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
#include <uxr/agent/transport/serial/SerialServerLinux.hpp>
#include <uxr/agent/transport/serial/baud_rate_table_linux.h>
#include <uxr/agent/transport/udp/UDPServerLinux.hpp>
#include <uxr/agent/transport/tcp/TCPServerLinux.hpp>
#include <termios.h>
#include <fcntl.h>
#endif //_WIN32
#include <iostream>
#include <string>
#include <sstream>
#include <limits>
#include <iterator>

void showHelp()
{
    std::cout << "Usage: program <command>" << std::endl;
    std::cout << "List of commands:" << std::endl;
#ifdef _WIN32
    std::cout << "    --udp <local_port> [--refs <refs-file>]" << std::endl;
    std::cout << "    --tcp <local_port> [--refs <refs-file>]" << std::endl;
#else
    std::cout << "    --serial <device-name> [--baudrate <baudrate>] [--refs <refs-file>]" << std::endl;
    std::cout << "    --pseudo-serial        [--baudrate <baudrate>] [--refs <refs-file>]" << std::endl;
    std::cout << "    --udp <local-port> [--discovery <discovery-port>] [--refs <refs-file>]" << std::endl;
    std::cout << "    --tcp <local-port> [--discovery <discovery-port>] [--refs <refs-file>]" << std::endl;
#endif
}

[[ noreturn ]] void initializationError()
{
    std::cout << "Error: Invalid arguments." << std::endl;
    showHelp();
    std::exit(EXIT_FAILURE);
}

uint16_t parsePort(const std::string& str_port)
{
    uint16_t valid_port = 0;
    try
    {
        int port = std::stoi(str_port);
        if(port > (std::numeric_limits<uint16_t>::max)())
        {
            std::cout << "Error: port number '" << port << "out of range." << std::endl;
            initializationError();
        }
        valid_port = uint16_t(port);
    }
    catch (const std::invalid_argument& )
    {
        initializationError();
    }
    return valid_port;
}

int main(int argc, char** argv)
{
    std::unique_ptr<eprosima::uxr::Server> server;
    std::vector<std::string> cl(0);

    if (1 == argc)
    {
        showHelp();
        std::cout << std::endl;
        std::cout << "Enter command: ";

        std::string raw_cl;
        std::getline(std::cin, raw_cl);
        std::istringstream iss(raw_cl);
        cl.insert(cl.begin(), std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>());
        std::cout << raw_cl << std::endl;
    }
    else
    {
        for (int i = 1; i < argc; ++i)
        {
            cl.push_back(argv[i]);
        }
    }

    auto it_cl = cl.begin();

    if ((cl.end() != it_cl) && (("-h" == *it_cl) || ("--help" == *it_cl)))
    {
        showHelp();
    }
    else if ((cl.end() != it_cl) && ("--udp" == *it_cl))
    {
        std::cout << "UDP agent initialization... ";
        uint16_t port = 0;
        if (cl.end() != ++it_cl)
        {
            port = parsePort(*(it_cl++));
        }
        else
        {
            initializationError();
        }
#ifdef _WIN32
        server.reset(new eprosima::uxr::UDPServer(port));
#else
        /*
         * Parse Discovery.
         */
        if ((cl.end() != it_cl) && ("--discovery" == *it_cl))
        {
            if (cl.end() != ++it_cl)
            {
                server.reset(new eprosima::uxr::UDPServer(port, parsePort(*(it_cl++))));
            }
            else
            {
                initializationError();
            }
        }
        else
        {
            server.reset(new eprosima::uxr::UDPServer(port));
        }

        /*
         * Parse Refs File.
         */
        if ((cl.end() != it_cl) && ("--refs" == *it_cl))
        {
            if ((cl.end() == ++it_cl) || !server->load_config_file(*(it_cl++)))
            {
                initializationError();
            }
        }
#endif
    }
    else if ((cl.end() != it_cl) && ("--tcp" == *it_cl))
    {
        std::cout << "TCP agent initialization... ";
        uint16_t port = 0;
        if (cl.end() != ++it_cl)
        {
            port = parsePort(*(it_cl++));
        }
        else
        {
            initializationError();
        }
#ifdef _WIN32
        server.reset(new eprosima::uxr::TCPServer(port));
#else
        /*
         * Parse Discovery.
         */
        if ((cl.end() != it_cl) && ("--discovery" == *it_cl))
        {
            if (cl.end() != ++it_cl)
            {
                server.reset(new eprosima::uxr::TCPServer(port, parsePort(*(it_cl++))));
            }
            else
            {
                initializationError();
            }
        }
        else
        {
            server.reset(new eprosima::uxr::TCPServer(port));
        }

        /*
         * Parse Refs File.
         */
        if ((cl.end() != it_cl) && ("--refs" == *it_cl))
        {
            if ((cl.end() == ++it_cl) || !server->load_config_file(*(it_cl++)))
            {
                initializationError();
            }
        }
#endif
    }
#ifndef _WIN32
    else if((cl.end() != it_cl) && ("--serial" == *it_cl))
    {
        std::cout << "Serial agent initialization... ";

        /* Open serial device. */
        if (cl.end() != ++it_cl)
        {
            int fd = open((it_cl++)->c_str(), O_RDWR | O_NOCTTY);
            if (0 < fd)
            {
                struct termios attr;
                memset(&attr, 0, sizeof(attr));
                if (0 == tcgetattr(fd, &attr))
                {
                    /* Setting CONTROL OPTIONS. */
                    attr.c_cflag |= CREAD;    // Enable read.
                    attr.c_cflag |= CLOCAL;   // Set local mode.
                    attr.c_cflag &= tcflag_t(~PARENB);  // Disable parity.
                    attr.c_cflag &= tcflag_t(~CSTOPB);  // Set one stop bit.
                    attr.c_cflag &= tcflag_t(~CSIZE);   // Mask the character size bits.
                    attr.c_cflag |= CS8;      // Set 8 data bits.
                    attr.c_cflag &= ~CRTSCTS; // Disable hardware flow control.

                    /* Setting LOCAL OPTIONS. */
                    attr.c_lflag &= tcflag_t(~ICANON);  // Set non-canonical input.
                    attr.c_lflag &= tcflag_t(~ECHO);    // Disable echoing of input characters.
                    attr.c_lflag &= tcflag_t(~ECHOE);   // Disable echoing the erase character.
                    attr.c_lflag &= tcflag_t(~ISIG);    // Disable SIGINTR, SIGSUSP, SIGDSUSP and SIGQUIT signals.

                    /* Setting INPUT OPTIONS. */
                    attr.c_iflag &= tcflag_t(~IXON);    // Disable output software flow control.
                    attr.c_iflag &= tcflag_t(~IXOFF);   // Disable input software flow control.
                    attr.c_iflag &= tcflag_t(~INPCK);   // Disable parity check.
                    attr.c_iflag &= tcflag_t(~ISTRIP);  // Disable strip parity bits.
                    attr.c_iflag &= tcflag_t(~IGNBRK);  // No ignore break condition.
                    attr.c_iflag &= tcflag_t(~IGNCR);   // No ignore carrier return.
                    attr.c_iflag &= tcflag_t(~INLCR);   // No map NL to CR.
                    attr.c_iflag &= tcflag_t(~ICRNL);   // No map CR to NL.

                    /* Setting OUTPUT OPTIONS. */
                    attr.c_oflag &= tcflag_t(~OPOST);   // Set raw output.

                    /* Setting OUTPUT CHARACTERS. */
                    attr.c_cc[VMIN] = 10;
                    attr.c_cc[VTIME] = 1;

                    /*
                     * Parse Baudrate.
                     */
                    speed_t baudrate = B115200;
                    if ((cl.end() != it_cl) && ("--baudrate" == *it_cl))
                    {
                        if (cl.end() != ++it_cl)
                        {
                            baudrate = getBaudRate((it_cl++)->c_str());
                        }
                        else
                        {
                            initializationError();
                        }
                    }

                    /* Setting BAUD RATE. */
                    cfsetispeed(&attr, baudrate);
                    cfsetospeed(&attr, baudrate);

                    if (0 == tcsetattr(fd, TCSANOW, &attr))
                    {
                        server.reset(new eprosima::uxr::SerialServer(fd, 0));
                    }

                    /*
                     * Parse Refs File.
                     */
                    if ((cl.end() != it_cl) && ("--refs" == *(it_cl++)))
                    {
                        if ((cl.end() == it_cl) || !server->load_config_file(*(it_cl++)))
                        {
                            initializationError();
                        }
                    }
                }
            }
            else
            {
                initializationError();
            }
        }
        else
        {
            initializationError();
        }
    }
    else if ((cl.end() != it_cl) && ("--pseudo-serial" == *it_cl))
    {
        ++it_cl;
        std::cout << "Pseudo-Serial initialization... ";

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

                /*
                 * Parse Baudrate.
                 */
                speed_t baudrate = B115200;
                if ((cl.end() != it_cl) && ("--baudrate" == *it_cl))
                {
                    if (cl.end() != ++it_cl)
                    {
                        baudrate = getBaudRate((it_cl++)->c_str());
                    }
                    else
                    {
                        initializationError();
                    }
                }

                /* Setting BAUD RATE. */
                cfsetispeed(&attr, baudrate);
                cfsetospeed(&attr, baudrate);

                if (0 == tcsetattr(fd, TCSANOW, &attr))
                {
                    std::cout << "Device: " << dev << std::endl;
                    server.reset(new eprosima::uxr::SerialServer(fd, 0));
                }

                /*
                 * Parse Refs File.
                 */
                if ((cl.end() != it_cl) && ("--refs" == *(it_cl++)))
                {
                    if ((cl.end() == it_cl) || !server->load_config_file(*(it_cl++)))
                    {
                        initializationError();
                    }
                }
            }
        }
    }
#endif
    else
    {
        initializationError();
    }

    if (nullptr != server)
    {
        /* Launch server. */
        if (server->run())
        {
            std::cout << "OK" << std::endl;
            std::cin.clear();
            char exit_flag = 0;
            while ('q' != exit_flag)
            {
                std::cout << "Enter 'q' for exit" << std::endl;
                std::cin >> exit_flag;
            }
            server->stop();
        }
        else
        {
            std::cout << "ERROR" << std::endl;
        }
    }

    return 0;
}
