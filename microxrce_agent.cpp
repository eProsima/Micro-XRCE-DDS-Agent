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
#include <limits>

void showHelp()
{
    std::cout << "Usage: program <command>" << std::endl;
    std::cout << "List of commands:" << std::endl;
    std::cout << "    serial <device_name>" << std::endl;
    std::cout << "    pseudo-serial" << std::endl;
    std::cout << "    udp <local_port> [<discovery_port>]" << std::endl;
    std::cout << "    tcp <local_port> [<discovery_port>]" << std::endl;
}

void initializationError()
{
    std::cout << "Error: Invalid arguments." << std::endl;
    showHelp();
    std::exit(EXIT_FAILURE);
}

uint16_t parsePort(const char* str_port)
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
    bool initialized = false;

#ifndef _WIN32
    if(argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0))
    {
        showHelp();
    }
    else if(argc >= 3 && strcmp(argv[1], "udp") == 0)
    {
        std::cout << "UDP agent initialization... ";
        uint16_t port = parsePort(argv[2]);
        eprosima::uxr::UDPServer* udp_server = (argc == 4) //discovery port
                                             ? new eprosima::uxr::UDPServer(port, parsePort(argv[3]))
                                             : new eprosima::uxr::UDPServer(port);
        if (udp_server->run())
        {
            initialized = true;
        }
        std::cout << ((!initialized) ? "ERROR" : "OK") << std::endl;
    }
    else if(argc >= 3 && strcmp(argv[1], "tcp") == 0)
    {
        std::cout << "TCP agent initialization... ";
        uint16_t port = parsePort(argv[2]);
        eprosima::uxr::TCPServer* tcp_server = (argc == 4) //discovery port
                                             ? new eprosima::uxr::TCPServer(port, parsePort(argv[3]))
                                             : new eprosima::uxr::TCPServer(port);
        if (tcp_server->run())
        {
            initialized = true;
        }
        std::cout << ((!initialized) ? "ERROR" : "OK") << std::endl;
    }
    else if(argc == 3 && strcmp(argv[1], "serial") == 0)
    {
        std::cout << "Serial agent initialization... ";

        /* Open serial device. */
        int fd = open(argv[2], O_RDWR | O_NOCTTY);
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
                cfsetispeed(&tty_config, B115200);
                cfsetospeed(&tty_config, B115200);

                if (0 == tcsetattr(fd, TCSANOW, &tty_config))
                {
                    eprosima::uxr::SerialServer* serial_server = new eprosima::uxr::SerialServer(fd, 0);
                    if (serial_server->run())
                    {
                        initialized = true;
                    }
                }
            }
        }

        std::cout << ((!initialized) ? "ERROR" : "OK") << std::endl;
    }
    else if (argc == 2 && strcmp(argv[1], "pseudo-serial") == 0)
    {
        std::cout << "Pseudo-Serial initialization... ";

        /* Open pseudo-terminal. */
        char* dev = NULL;
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
            }
        }

        /* Launch server. */
        eprosima::uxr::SerialServer* serial_server = new eprosima::uxr::SerialServer(fd, 0x00);
        if (serial_server->run())
        {
            initialized = true;
        }

        if (initialized)
        {
            std::cout << "OK" << std::endl;
            std::cout << "Device: " << dev << std::endl;
        }
        else
        {
            std::cout << "ERROR" << std::endl;
        }
    }
    else
    {
        initializationError();
    }
#else
    (void) argc;
    (void) argv;

    std::string server_type = "";
    std::cout << "Select server type [udp|tcp]: ";
    std::cin >> server_type;

    if ("udp" == server_type)
    {
        uint16_t port = 0;
        std::cout << "Select port: ";
        std::cin >> port;

        std::cout << "UDP agent initialization.... ";
        eprosima::uxr::UDPServer* udp_server = new eprosima::uxr::UDPServer(port);
        if (udp_server->run())
        {
            initialized = true;
        }
        std::cout << ((!initialized) ? "ERROR" : "OK") << std::endl;
    }
    else if ("tcp" == server_type)
    {
        uint16_t port = 0;
        std::cout << "Select port: ";
        std::cin >> port;

        std::cout << "UDP agent initialization.... ";
        eprosima::uxr::TCPServer* tcp_server = new eprosima::uxr::TCPServer(port);
        if (tcp_server->run())
        {
            initialized = true;
        }
        std::cout << ((!initialized) ? "ERROR" : "OK") << std::endl;
    }
    else
    {
        std::cout << "Error server type" << std::endl;
    }

#endif

    if(initialized)
    {
        std::cin.clear();
        char exit_flag = 0;
        while ('q' != exit_flag)
        {
            std::cout << "Enter 'q' for exit" << std::endl;
            std::cin >> exit_flag;
        }
    }

    return 0;
}

