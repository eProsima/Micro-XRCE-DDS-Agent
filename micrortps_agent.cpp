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
#define _XOPEN_SOURCE >= 600
#include <termios.h>

#include <agent/Root.h>

int main(int argc, char** argv)
{
    bool initialized = false;

    eprosima::micrortps::Agent& micrortps_agent = eprosima::micrortps::root();
    if(argc == 3 && strcmp(argv[1], "uart") == 0)
    {
        std::cout << "UART agent initialization... ";
        initialized = micrortps_agent.init(argv[2]);
        std::cout << ((!initialized) ? "ERROR" : "OK") << std::endl;
    }
    else if (argc == 2 && strcmp(argv[1], "pseudo-uart") == 0)
    {
        std::cout << "Pseudo-UART initialization... ";

        /* Open pseudo-terminal. */
        char* dev;
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
                initialized = micrortps_agent.init(fd, 0x00);
            }
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
    else if(argc ==3 && strcmp(argv[1], "udp") == 0)
    {
        std::cout << "UDP agent initialization... ";
        initialized = micrortps_agent.init((uint16_t)atoi(argv[2]));
        std::cout << ((!initialized) ? "ERROR" : "OK") << std::endl;
    }
    else
    {
        std::cout << "Error: Invalid arguments." << std::endl;
    }

    if(initialized)
    {
        micrortps_agent.run();
    }
    else
    {
        std::cout << "Help: program <command>" << std::endl;
        std::cout << "List of commands:" << std::endl;
        std::cout << "    serial <device>" << std::endl;
        std::cout << "    udp <local_port>" << std::endl;
    }
    return 0;
}
