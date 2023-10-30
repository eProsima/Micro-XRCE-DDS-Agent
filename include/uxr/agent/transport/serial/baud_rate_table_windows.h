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

inline
DWORD getBaudRate(const char* baudrate_str)
{
    DWORD rv;
    if (0 == strcmp(baudrate_str, "0"))
    {
        rv = 0;
    }
    else if (0 == strcmp(baudrate_str, "110"))
    {
        rv = CBR_110;
    }
    else if (0 == strcmp(baudrate_str, "300")) {
        rv = CBR_300;
    }
    else if (0 == strcmp(baudrate_str, "600")) {
        rv = CBR_600;
    }
    else if (0 == strcmp(baudrate_str, "1200")) {
        rv = CBR_1200;
    }
    else if (0 == strcmp(baudrate_str, "2400")) {
        rv = CBR_2400;
    }
    else if (0 == strcmp(baudrate_str, "4800")) {
        rv = CBR_4800;
    }
    else if (0 == strcmp(baudrate_str, "9600")) {
        rv = CBR_9600;
    }
    else if (0 == strcmp(baudrate_str, "14400")) {
        rv = CBR_14400;
    }
    else if (0 == strcmp(baudrate_str, "19200")) {
        rv = CBR_19200;
    }
    else if (0 == strcmp(baudrate_str, "38400")) {
        rv = CBR_38400;
    }
    else if (0 == strcmp(baudrate_str, "56000")) {
        rv = CBR_56000;
    }
    else if (0 == strcmp(baudrate_str, "57600")) {
        rv = CBR_57600;
    }
    else if (0 == strcmp(baudrate_str, "115200")) {
        rv = CBR_115200;
    }
    else if (0 == strcmp(baudrate_str, "128000")) {
        rv = CBR_128000;
    }
    else if (0 == strcmp(baudrate_str, "256000")) {
        rv = CBR_256000;
    }
    else
    {
        DWORD custom_baud_rate = (DWORD)atoi(baudrate_str);
        printf("Warning: Custom baud rate set to: %d\n", custom_baud_rate);
        rv = custom_baud_rate;
    }
    return rv;
}

#endif // UXR_AGENT_TRANSPORT_SERIAL_BAUD_RATE_TABLE_H

