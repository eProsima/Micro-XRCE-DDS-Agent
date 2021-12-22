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

#ifndef UXR_AGENT_TRANSPORT_SERIAL_BAUD_RATE_TABLE_H
#define UXR_AGENT_TRANSPORT_SERIAL_BAUD_RATE_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

inline
speed_t getBaudRate(const char* baudrate_str)
{
    speed_t rv;
    if (0 == strcmp(baudrate_str, "0"))
    {
        rv = B0;
    }
    else if (0 == strcmp(baudrate_str, "50"))
    {
        rv = B50;
    }
    else if (0 == strcmp(baudrate_str, "75"))
    {
        rv = B75;
    }
    else if (0 == strcmp(baudrate_str, "110"))
    {
        rv = B110;
    }
    else if (0 == strcmp(baudrate_str, "134"))
    {
        rv = B134;
    }
    else if (0 == strcmp(baudrate_str, "150"))
    {
        rv = B150;
    }
    else if (0 == strcmp(baudrate_str, "200"))
    {
        rv = B200;
    }
    else if (0 == strcmp(baudrate_str, "300"))
    {
        rv = B300;
    }
    else if (0 == strcmp(baudrate_str, "600"))
    {
        rv = B600;
    }
    else if (0 == strcmp(baudrate_str, "1200"))
    {
        rv = B1200;
    }
    else if (0 == strcmp(baudrate_str, "1800"))
    {
        rv = B1800;
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
    else if (0 == strcmp(baudrate_str, "230400"))
    {
        rv = B230400;
    }
#ifndef __APPLE__
    else if (0 == strcmp(baudrate_str, "460800"))
    {
        rv = B460800;
    }
    else if (0 == strcmp(baudrate_str, "500000"))
    {
        rv = B500000;
    }
    else if (0 == strcmp(baudrate_str, "576000"))
    {
        rv = B576000;
    }
    else if (0 == strcmp(baudrate_str, "921600"))
    {
        rv = B921600;
    }
    else if (0 == strcmp(baudrate_str, "1000000"))
    {
        rv = B1000000;
    }
    else if (0 == strcmp(baudrate_str, "1152000"))
    {
        rv = B1152000;
    }
    else if (0 == strcmp(baudrate_str, "1500000"))
    {
        rv = B1500000;
    }
    else if (0 == strcmp(baudrate_str, "2000000"))
    {
        rv = B2000000;
    }
    else if (0 == strcmp(baudrate_str, "2500000"))
    {
        rv = B2500000;
    }
    else if (0 == strcmp(baudrate_str, "3000000"))
    {
        rv = B3000000;
    }
    else if (0 == strcmp(baudrate_str, "3500000"))
    {
        rv = B3500000;
    }
    else if (0 == strcmp(baudrate_str, "4000000"))
    {
        rv = B4000000;
    }
#endif // __APPLE__
    else
    {
        speed_t custom_baud_rate = (speed_t)atoi(baudrate_str);
        printf("Warning: Custom baud rate set to: %d\n", custom_baud_rate);
        rv = custom_baud_rate;
    }
    return rv;
}

#endif // UXR_AGENT_TRANSPORT_SERIAL_BAUD_RATE_TABLE_H

