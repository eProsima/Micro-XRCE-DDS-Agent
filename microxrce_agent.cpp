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
    std::cout << "    udp <local_port>" << std::endl;
    std::cout << "    tcp <local_port>" << std::endl;
#else
    std::cout << "    serial <device_name> [baudrate=<1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200>]" << std::endl;
    std::cout << "    pseudo-serial [baudrate=<1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200>]" << std::endl;
    std::cout << "    udp <local_port> [<discovery_port>]" << std::endl;
    std::cout << "    tcp <local_port> [<discovery_port>]" << std::endl;
#endif
}

speed_t getBaudRate(const char* baudrate_str)
{
    speed_t rv;
    if (0 == strcmp(baudrate_str, "1200"))
    {
        rv = B1200;
    }
    else if (0 == strcmp(baudrate_str, "2400"))
    {
        rv = B2400;
    }
    else if (0 == strcmp(baudrate_str, "4800"))
    {
        rv = B4800;
    }
    else if (0 == strcmp(baudrate_str, "9600"))
    {
        rv = B9600;
    }
    else if (0 == strcmp(baudrate_str, "19200"))
    {
        rv = B19200;
    }
    else if (0 == strcmp(baudrate_str, "38400"))
    {
        rv = B38400;
    }
    else if (0 == strcmp(baudrate_str, "57600"))
    {
        rv = B57600;
    }
    else if (0 == strcmp(baudrate_str, "115200"))
    {
        rv = B115200;
    }
    else
    {
        rv = 0;
    }
    return rv;
}

void initializationError()
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
    eprosima::uxr::Server* server = nullptr;
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

    if((1 == cl.size()) && (("-h" == cl[0]) || ("--help" == cl[0])))
    {
        showHelp();
    }
    else if((2 <= cl.size()) && ("udp" == cl[0]))
    {
        std::cout << "UDP agent initialization... ";
        uint16_t port = parsePort(cl[1]);
#ifdef _WIN32
        server = new eprosima::uxr::UDPServer(port);
#else
        server = (3 == cl.size()) //discovery port
                 ? new eprosima::uxr::UDPServer(port, parsePort(cl[2]))
                 : new eprosima::uxr::UDPServer(port);
#endif
    }
    else if((2 <= cl.size()) && ("tcp" == cl[0]))
    {
        std::cout << "TCP agent initialization... ";
        uint16_t port = parsePort(cl[1]);
#ifdef _WIN32
        server = new eprosima::uxr::TCPServer(port);
#else
        server = (3 == cl.size()) //discovery port
                 ? new eprosima::uxr::TCPServer(port, parsePort(cl[2]))
                 : new eprosima::uxr::TCPServer(port);
#endif
    }
#ifndef _WIN32
    else if((2 <= cl.size()) && ("serial" == cl[0]))
    {
        std::cout << "Serial agent initialization... ";

        /* Open serial device. */
        int fd = open(cl[1].c_str(), O_RDWR | O_NOCTTY);
        if (0 < fd)
        {
            struct termios tty_config;
            memset(&tty_config, 0, sizeof(tty_config));
            if (0 == tcgetattr(fd, &tty_config))
            {
                /* Setting CONTROL OPTIONS. */
                tty_config.c_cflag |= CREAD;    // Enable read.
                tty_config.c_cflag |= CLOCAL;   // Set local mode.
                tty_config.c_cflag &= ~PARENB;  // Disable parity.
                tty_config.c_cflag &= ~CSTOPB;  // Set one stop bit.
                tty_config.c_cflag &= ~CSIZE;   // Mask the character size bits.
                tty_config.c_cflag |= CS8;      // Set 8 data bits.
                tty_config.c_cflag &= ~CRTSCTS; // Disable hardware flow control.

                /* Setting LOCAL OPTIONS. */
                tty_config.c_lflag &= ~ICANON;  // Set non-canonical input.
                tty_config.c_lflag &= ~ECHO;    // Disable echoing of input characters.
                tty_config.c_lflag &= ~ECHOE;   // Disable echoing the erase character.
                tty_config.c_lflag &= ~ISIG;    // Disable SIGINTR, SIGSUSP, SIGDSUSP and SIGQUIT signals.

                /* Setting INPUT OPTIONS. */
                tty_config.c_iflag &= ~IXON;    // Disable output software flow control.
                tty_config.c_iflag &= ~IXOFF;   // Disable input software flow control.
                tty_config.c_iflag &= ~INPCK;   // Disable parity check.
                tty_config.c_iflag &= ~ISTRIP;  // Disable strip parity bits.
                tty_config.c_iflag &= ~IGNBRK;  // No ignore break condition.
                tty_config.c_iflag &= ~IGNCR;   // No ignore carrier return.
                tty_config.c_iflag &= ~INLCR;   // No map NL to CR.
                tty_config.c_iflag &= ~ICRNL;   // No map CR to NL.

                /* Setting OUTPUT OPTIONS. */
                tty_config.c_oflag &= ~OPOST;   // Set raw output.

                /* Setting OUTPUT CHARACTERS. */
                tty_config.c_cc[VMIN] = 10;
                tty_config.c_cc[VTIME] = 1;

                /* Setting BAUD RATE. */
                speed_t baudrate = (3 == cl.size()) ? getBaudRate(cl[2].c_str()) : B115200;
                if (0 != baudrate)
                {
                    cfsetispeed(&tty_config, baudrate);
                    cfsetospeed(&tty_config, baudrate);

                    if (0 == tcsetattr(fd, TCSANOW, &tty_config))
                    {
                        server = new eprosima::uxr::SerialServer(fd, 0);
                    }
                }
            }
        }
    }
    else if ((1 <= cl.size()) && ("pseudo-serial" == cl[0]))
    {
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

                /* Setting BAUD RATE. */
                speed_t baudrate = (2 == cl.size()) ? getBaudRate(cl[1].c_str()) : B115200;
                if (0 != baudrate)
                {
                    cfsetispeed(&attr, baudrate);
                    cfsetospeed(&attr, baudrate);

                    if (0 == tcsetattr(fd, TCSANOW, &attr))
                    {
                        std::cout << "Device: " << dev << std::endl;
                        server = new eprosima::uxr::SerialServer(fd, 0x00);
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
        delete server;
    }

    return 0;
}

